/*
    SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "plistreader_p.h"
#include "plistdata_p.h"

#include <QDebug>
#include <QJsonArray>
#include <QJsonObject>
#include <QStringDecoder>

#include <cassert>
#include <cstring>

using namespace KItinerary;

PListArray::PListArray() = default;
PListArray::PListArray(std::string_view data, const PListReader *reader) : m_data(data), m_reader(reader) {}
PListArray::~PListArray() = default;

uint64_t PListArray::size() const
{
    return m_reader ? (m_data.size() / m_reader->trailer()->objectRefSize) : 0;
}

uint64_t PListArray::value(uint64_t index) const
{
    return m_reader ? m_reader->readObjectRef(m_data, index) : 0;
}

PListObjectType PListArray::objectType(uint64_t index) const
{
    return m_reader ? m_reader->objectType(value(index)) : PListObjectType::Unused;
}
PListObjectType PListArray::objectType(PListUid uid) const { return objectType(uid.value); }

QVariant PListArray::object(uint64_t index) const
{
    return m_reader ? m_reader->object(value(index)) : QVariant();
}
QVariant PListArray::object(PListUid uid) const { return object(uid.value); }


PListDict::PListDict() = default;
PListDict::PListDict(std::string_view data, const PListReader *reader) : m_data(data), m_reader(reader) {}
PListDict::~PListDict() = default;

uint64_t PListDict::size() const
{
    return m_reader ? (m_data.size() / (m_reader->trailer()->objectRefSize * 2)) : 0;
}

uint64_t PListDict::key(uint64_t index) const
{
    return m_reader ? m_reader->readObjectRef(m_data, index) : 0;
}

uint64_t PListDict::value(uint64_t index) const
{
    return m_reader ? m_reader->readObjectRef(m_data, size() + index) : 0;
}

std::optional<uint64_t> PListDict::value(QLatin1StringView keyName) const {
  if (keyName.isEmpty()) {
    return {};
  }

  for (uint64_t i = 0; i < size(); ++i) {
    const auto n = m_reader->object(key(i)).toString();
    if (n == keyName) {
      return value(i);
    }
  }

  return {};
}

QVariant PListDict::object(QLatin1StringView keyName) const {
  const auto v = value(keyName);
  return v && m_reader ? m_reader->object(*v) : QVariant();
}

PListReader::PListReader(const QByteArray &data)
{
    if (!maybePList(data)) {
        return;
    }

    m_data = data;
}

PListReader::~PListReader() = default;

bool PListReader::isValid() const
{
    return !m_data.isEmpty();
}

uint64_t PListReader::objectCount() const
{
    return trailer()->numObjects;
}

uint64_t PListReader::rootObjectIndex() const
{
    return trailer()->rootObjectIndex;
}

uint64_t PListReader::objectOffset(uint64_t index) const
{
    const auto t = trailer();
    if (!t || index >= t->numObjects) {
        return 0;
    }

    uint64_t offset = 0;
    for (uint64_t i = 0; i < t->offsetIntSize; ++i) {
        offset <<= 8;
        offset |= (uint8_t)m_data.at(t->offsetTableOffset + index * t->offsetIntSize + i);
    }
    return offset;
}

struct {
    uint8_t marker;
    uint8_t mask;
    PListObjectType type;
}
static constexpr const marker_map[] = {
    { 0b0000'0000, 0b1111'1111, PListObjectType::Null },
    { 0b0000'1000, 0b1111'1110, PListObjectType::Bool },
    { 0b0000'1100, 0b1111'1110, PListObjectType::Url },
    { 0b0000'1110, 0b1111'1111, PListObjectType::Uuid },
    { 0b0000'1111, 0b1111'1111, PListObjectType::Fill },
    { 0b0001'0000, 0b1111'1000, PListObjectType::Int },
    { 0b0010'0000, 0b1111'1000, PListObjectType::Real },
    { 0b0011'0011, 0b1111'1111, PListObjectType::Date },
    { 0b0100'0000, PListContainerTypeMask, PListObjectType::Data },
    { 0b0101'0000, PListContainerTypeMask, PListObjectType::String },
    { 0b0110'0000, 0b1110'0000, PListObjectType::String },
    { 0b1000'0000, 0b1111'0000, PListObjectType::Uid },
    { 0b1010'0000, PListContainerTypeMask, PListObjectType::Array },
    { 0b1011'0000, PListContainerTypeMask, PListObjectType::Ordset },
    { 0b1100'0000, PListContainerTypeMask, PListObjectType::Set },
    { 0b1101'0000, PListContainerTypeMask, PListObjectType::Dict },
};

static PListObjectType objectTypeFromMarker(uint8_t b)
{
    for (const auto &m : marker_map) {
        if ((b & m.mask) == m.marker) {
            return m.type;
        }
    }
    return PListObjectType::Unused;
}

PListObjectType PListReader::objectType(uint64_t index) const
{
    const auto offset = objectOffset(index);
    if (offset >= (uint64_t)m_data.size()) {
        return PListObjectType::Unused;
    }

    return objectTypeFromMarker(m_data.at(offset));
}

QVariant PListReader::object(uint64_t index) const
{
    auto offset = objectOffset(index);
    if (offset >= (uint64_t)m_data.size()) {
        return {};
    }

    const uint8_t b = m_data.at(offset++);
    const auto type = objectTypeFromMarker(b);
    switch (type) {
        case PListObjectType::Null:
        case PListObjectType::Fill:
        case PListObjectType::Unused:
            return {};
        case PListObjectType::Bool:
            return b == PListTrue;
        case PListObjectType::Int:
            return QVariant::fromValue<quint64>(readBigEndianNumber(offset, 1 << (b & 0b0000'0111)));
        case PListObjectType::Data:
        case PListObjectType::String:
        {
            if ((b & PListContainerTypeMask) == 0b0110'0000) {
                const auto size = readContainerSize(offset, b) * 2;
                const auto v = view(offset, size);
                auto codec = QStringDecoder(QStringDecoder::Utf16BE);
                return QString(codec.decode(QByteArrayView(v.data(), v.size())));
            }

            const auto size = readContainerSize(offset, b);
            const auto v = view(offset, size);
            if ((b & PListContainerTypeMask) == 0b0101'0000) {
                return QString::fromLatin1(v.data(), v.size());
            }
            if ((b & PListContainerTypeMask) == 0b0111'0000) {
                return QString::fromUtf8(v.data(), v.size());
            }
            return QByteArray(v.data(), v.size());
        }
        case PListObjectType::Uid:
            return QVariant::fromValue(PListUid{readBigEndianNumber(offset, (b & 0b0000'1111) + 1)});
        case PListObjectType::Array:
        {
            const auto size = readContainerSize(offset, b) * trailer()->objectRefSize;
            return QVariant::fromValue(PListArray(view(offset, size), this));
        }
        case PListObjectType::Dict:
        {
            const auto size = readContainerSize(offset, b) * trailer()->objectRefSize * 2;
            return QVariant::fromValue(PListDict(view(offset, size), this));
        }

        case PListObjectType::Real:
        case PListObjectType::Date:
            // TODO

        // v1+ types we don't support/need yet
        case PListObjectType::Url:
        case PListObjectType::Uuid:
        case PListObjectType::Ordset:
        case PListObjectType::Set:
            qDebug() << "unsupposed plist object type:" << (int)type << (int)b;
            break;
    }
    return {};
}

const PListTrailer* PListReader::trailer() const
{
    return isValid() ? reinterpret_cast<const PListTrailer*>(m_data.constData() + m_data.size() - sizeof(PListTrailer)) : nullptr;
}

uint64_t PListReader::readBigEndianNumber(uint64_t offset, int size) const
{
    if (size >= 8) {
        qDebug() << "oversized int not supported" << offset << size;
        return {};
    }
    if ((uint64_t)m_data.size() <= offset + size) {
        qDebug() << "attempting to read number beyond input data" << offset << size;
        return {};
    }

    uint64_t v = 0;
    for (auto i = 0; i < size; ++i) {
        v <<= 8;
        v |= (uint8_t)m_data.at(offset + i);
    }
    return v;
}

uint64_t PListReader::readBigEndianInteger(uint64_t& offset) const
{
    assert(offset < (uint64_t)m_data.size());
    const auto b = m_data.at(offset++);
    const auto size = 1 << (b & 0b0000'0111);
    const auto v = readBigEndianNumber(offset, size);
    offset += size;
    return v;
}

uint64_t PListReader::readContainerSize(uint64_t &offset, uint8_t marker) const
{
    uint64_t size = (marker & PListContainerSizeMask);
    if (offset + size >= (uint64_t)m_data.size()) {
        qDebug() << "attempt to read data beyond bounds";
        return {};
    }

    if (size == PListContainerSizeMask) {
        size = readBigEndianInteger(offset);
    }
    return size;
}

std::string_view PListReader::view(uint64_t offset, uint64_t size) const
{
    return offset + size < (uint64_t)m_data.size() ? std::string_view(m_data.constData() + offset, size) : std::string_view();
}

uint64_t PListReader::readObjectRef(std::string_view data, uint64_t index) const
{
    if ((index + 1) * trailer()->objectRefSize > data.size()) {
        qDebug() << "object reference read beyond data size";
        return {};
    }

    uint64_t ref = 0;
    for (auto i = 0; i < trailer()->objectRefSize; ++i) {
        ref <<= 8;
        ref |= (uint8_t)data[index * trailer()->objectRefSize + i];
    }
    return ref;
}

QJsonValue PListReader::unpackKeyedArchive() const
{
    // TODO cycle detection
    const auto root = object(rootObjectIndex()).value<PListDict>();
    if (root.object(QLatin1StringView("$archiver")).toString() !=
        QLatin1StringView("NSKeyedArchiver")) {
      qDebug() << "not NSKeyedArchiver data"
               << root.object(QLatin1StringView("$archiver"));
      return {};
    }

    const auto top = root.object(QLatin1StringView("$top")).value<PListDict>();
    const auto objects =
        root.object(QLatin1StringView("$objects")).value<PListArray>();
    const auto uid = top.object(QLatin1StringView("root")).value<PListUid>();
    return unpackKeyedArchiveRecursive(uid, objects);
}

QJsonValue PListReader::unpackKeyedArchiveRecursive(PListUid uid, const PListArray &objects) const
{
    const auto type = objects.objectType(uid);
    const auto v = objects.object(uid);
    switch (type) {
        case PListObjectType::Dict:
        {
            const auto obj = v.value<PListDict>();
            const auto classObj =
                objects
                    .object(obj.object(QLatin1StringView("$class"))
                                .value<PListUid>())
                    .value<PListDict>();
            const auto classNames =
                classObj.object(QLatin1StringView("$classes"))
                    .value<PListArray>();
            for (uint64_t j = 0; j < classNames.size(); ++j) {
                const auto className = classNames.object(j).toString();
                if (className == QLatin1StringView("NSDictionary")) {
                  QJsonObject result;
                  const auto keys = obj.object(QLatin1StringView("NS.keys"))
                                        .value<PListArray>();
                  const auto vals = obj.object(QLatin1StringView("NS.objects"))
                                        .value<PListArray>();
                  for (uint64_t i = 0; i < std::min(keys.size(), vals.size());
                       ++i) {
                    const auto key =
                        objects.object(keys.object(i).value<PListUid>())
                            .toString();
                    const auto valUid = vals.object(i).value<PListUid>();
                    result.insert(key,
                                  unpackKeyedArchiveRecursive(valUid, objects));
                  }
                  return result;
                }
                if (className == QLatin1StringView("NSArray")) {
                  QJsonArray result;
                  const auto elems = obj.object(QLatin1StringView("NS.objects"))
                                         .value<PListArray>();
                  for (uint64_t i = 0; i < elems.size(); ++i) {
                    const auto elemUid = elems.object(i).value<PListUid>();
                    result.push_back(
                        unpackKeyedArchiveRecursive(elemUid, objects));
                  }
                  return result;
                }
            }
            qDebug() << "unhandled dict object" << uid.value;
            break;
        }
        case PListObjectType::Int:
            return v.toInt();
        case PListObjectType::String:
            return v.toString();
        default:
            qDebug() << "unhandled class" << (int)type;
            return {};
    }

    return {};
}

bool PListReader::maybePList(const QByteArray &data)
{
    if ((std::size_t)data.size() <= sizeof(PListHeader) + sizeof(PListReader)) {
        return false;
    }

    const auto header = reinterpret_cast<const PListHeader*>(data.constData());
    if (std::memcmp(header->magic, PListMagic, PListMagicSize) != 0) {
        return false;
    }
    qDebug() << "found plist version:" << header->version[0] << header->version[1];

    // verify trailer content
    const auto t = reinterpret_cast<const PListTrailer*>(data.constData() + data.size() - sizeof(PListTrailer));
    if (t->offsetIntSize == 0 || t->offsetIntSize > 8 || t->objectRefSize == 0 || t->objectRefSize > 8) {
        return false;
    }

    return t->offsetTableOffset + t->numObjects * t->offsetIntSize < (uint64_t)data.size();
}
