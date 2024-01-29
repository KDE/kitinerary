/*
    SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KITINERARY_PLISTREADER_P_H
#define KITINERARY_PLISTREADER_P_H

#include <QByteArray>
#include <QMetaType>

#include <cstdint>
#include <optional>
#include <string_view>

class QVariant;

namespace KItinerary {

class PListReader;
struct PListTrailer;
enum class PListObjectType;

/** Property list UID object type. */
struct PListUid
{
public:
    uint64_t value;
};


/** Property list array objects. */
class PListArray
{
public:
    explicit PListArray();
    ~PListArray();

    uint64_t size() const;
    uint64_t value(uint64_t index) const;
    PListObjectType objectType(uint64_t index) const;
    PListObjectType objectType(PListUid uid) const;
    QVariant object(uint64_t index) const;
    QVariant object(PListUid uid) const;

private:
    friend class PListReader;
    explicit PListArray(std::string_view data, const PListReader *reader);
    std::string_view m_data;
    const PListReader *m_reader = nullptr;
};

/** Property list dictionary objects. */
class PListDict
{
public:
    explicit PListDict();
    ~PListDict();

    uint64_t size() const;
    uint64_t key(uint64_t index) const;
    uint64_t value(uint64_t index) const;
    std::optional<uint64_t> value(QLatin1StringView keyName) const;
    QVariant object(QLatin1StringView keyName) const;

  private:
    friend class PListReader;
    explicit PListDict(std::string_view data, const PListReader *reader);
    std::string_view m_data;
    const PListReader *m_reader = nullptr;
};

/**
 * Reading of Apple binary property list data.
 * @see https://en.wikipedia.org/wiki/Property_list
 */
class PListReader
{
public:
    explicit PListReader(const QByteArray &data = {});
    ~PListReader();

    bool isValid() const;

    /** Number of objects in the property list. */
    uint64_t objectCount() const;
    /** Root object index. */
    uint64_t rootObjectIndex() const;
    /** Data type of the object at @p index. */
    PListObjectType objectType(uint64_t index) const;
    QVariant object(uint64_t index) const;

    /** Unpack NSKeyedArchiver data to JSON. */
    QJsonValue unpackKeyedArchive() const;

    /** Check whether the given data might be a binary property list. */
    static bool maybePList(const QByteArray &data);

private:
    friend class PListArray;
    friend class PListDict;

    const PListTrailer* trailer() const;
    /** Offset into the data for object at @p index. */
    uint64_t objectOffset(uint64_t index) const;
    uint64_t readBigEndianNumber(uint64_t offset, int size) const;
    uint64_t readBigEndianInteger(uint64_t &offset) const;
    uint64_t readContainerSize(uint64_t &offset, uint8_t marker) const;
    std::string_view view(uint64_t offset, uint64_t size) const;
    uint64_t readObjectRef(std::string_view data, uint64_t index) const;

    QJsonValue unpackKeyedArchiveRecursive(PListUid uid, const PListArray &objects) const;

    QByteArray m_data;
};

}

Q_DECLARE_METATYPE(KItinerary::PListUid)
Q_DECLARE_METATYPE(KItinerary::PListArray)
Q_DECLARE_METATYPE(KItinerary::PListDict)
Q_DECLARE_METATYPE(KItinerary::PListReader)

#endif // KITINERARY_PLISTREADER_P_H
