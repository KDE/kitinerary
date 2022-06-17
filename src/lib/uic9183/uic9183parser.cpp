/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "uic9183parser.h"
#include "logging.h"
#include "rct2ticket.h"
#include "uic9183block.h"
#include "uic9183head.h"
#include "uic9183header.h"
#include "uic9183ticketlayout.h"
#include "vendor0080block.h"

#include <QDateTime>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>

#include <zlib.h>

#include <cassert>
#include <cstring>

using namespace KItinerary;

namespace KItinerary {

class Uic9183ParserPrivate : public QSharedData
{
public:
    QByteArray m_data;
    QByteArray m_payload;
};
}

Uic9183Parser::Uic9183Parser()
    : d(new Uic9183ParserPrivate)
{
}

Uic9183Parser::Uic9183Parser(const Uic9183Parser&) = default;
Uic9183Parser::~Uic9183Parser() = default;
Uic9183Parser& Uic9183Parser::operator=(const Uic9183Parser&) = default;

Uic9183Block Uic9183Parser::firstBlock() const
{
    return Uic9183Block(d->m_payload, 0);
}

Uic9183Block Uic9183Parser::findBlock(const char name[6]) const
{
    for (auto block = firstBlock(); !block.isNull(); block = block.nextBlock()) {
        if (block.isA(name)) {
            return block;
        }
    }
    return {};
}

QVariant Uic9183Parser::block(const QString &name) const
{
    if (name.size() != 6 || d->m_payload.isEmpty()) {
        return {};
    }

    if (name == QLatin1String(Uic9183Head::RecordId)) {
        return QVariant::fromValue(findBlock<Uic9183Head>());
    }
    if (name == QLatin1String("0080BL")) {
        return QVariant::fromValue(findBlock<Vendor0080BLBlock>());
    }
    if (name == QLatin1String("0080VU")) {
        return QVariant::fromValue(findBlock<Vendor0080VUBlock>());
    }
    return QVariant::fromValue(findBlock(name.toUtf8().constData()));
}

void Uic9183Parser::setContextDate(const QDateTime&)
{
}

void Uic9183Parser::parse(const QByteArray &data)
{
    d->m_data.clear();
    d->m_payload.clear();

    Uic9183Header header(data);
    if (!header.isValid()) {
        return;
    }

    // nx zlib payload
    d->m_data = data;
    d->m_payload.resize(4096);
    z_stream stream;
    stream.zalloc = nullptr;
    stream.zfree = nullptr;
    stream.opaque = nullptr;
    stream.avail_in = data.size() - header.compressedMessageOffset();
    stream.next_in = reinterpret_cast<unsigned char*>(const_cast<char*>(data.data() + header.compressedMessageOffset()));
    stream.avail_out = d->m_payload.size();
    stream.next_out = reinterpret_cast<unsigned char*>(d->m_payload.data());

    inflateInit(&stream);
    const auto res = inflate(&stream, Z_NO_FLUSH);
    switch (res) {
        case Z_OK:
        case Z_STREAM_END:
            break; // all good
        default:
            qCWarning(Log) << "UIC 918.3 payload zlib decompression failed" << stream.msg;
            return;
    }
    inflateEnd(&stream);
    d->m_payload.truncate(d->m_payload.size() - stream.avail_out);
    //qCDebug(Log) << res <<  d->m_payload << stream.avail_out;
}

bool Uic9183Parser::isValid() const
{
    return !d->m_payload.isEmpty();
}

QString Uic9183Parser::pnr() const
{
    const auto key = findBlock<Uic9183Head>().ticketKey();
    return key.startsWith(QLatin1Char(' ')) ? key.trimmed() : key.left(6);
}

QString Uic9183Parser::name() const
{
    // DB vendor block
    const auto b = findBlock<Vendor0080BLBlock>();
    if (b.isValid()) {
        const auto sblock = b.findSubBlock("001");
        if (!sblock.isNull()) {
            return QString::fromUtf8(sblock.content(), sblock.contentSize());
        }
    }

    // RCT2
    const auto rct2 = rct2Ticket();
    if (rct2.isValid()) {
        return rct2.title();
    }

    return {};
}

QString Uic9183Parser::carrierId() const
{
    return findBlock<Uic9183Head>().issuerCompanyCodeString();
}

QDateTime Uic9183Parser::validFrom() const
{
    // DB vendor block
    if (const auto b = findBlock<Vendor0080BLBlock>(); b.isValid() && b.orderBlockCount() == 1) {
        return QDateTime(b.orderBlock(0).validFrom(), {0, 0, 0});
    }

    // ÖBB vender block
    if (const auto b = findBlock("118199"); !b.isNull()) {
        const auto obj = QJsonDocument::fromJson(QByteArray::fromRawData(b.content(), b.contentSize())).object();
        auto dt = QDateTime::fromString(obj.value(QLatin1String("V")).toString(), QStringLiteral("yyMMddhhmm"));
        if (dt.date().year() < 2000) {
            dt = dt.addYears(100);
        }
        dt.setTimeSpec(Qt::UTC);
        return dt;
    }

    // RCT2
    if (const auto  rct2 = rct2Ticket(); rct2.isValid()) {
        return QDateTime(rct2.firstDayOfValidity(), {0, 0, 0});
    }

    return {};
}

QDateTime Uic9183Parser::validUntil() const
{
    // DB vendor block
    const auto b = findBlock<Vendor0080BLBlock>();
    if (b.isValid() && b.orderBlockCount() == 1) {
        return QDateTime(b.orderBlock(0).validTo(), {23, 59, 59});
    }

    // ÖBB vender block
    if (const auto b = findBlock("118199"); !b.isNull()) {
        const auto obj = QJsonDocument::fromJson(QByteArray::fromRawData(b.content(), b.contentSize())).object();
        auto dt = QDateTime::fromString(obj.value(QLatin1String("B")).toString(), QStringLiteral("yyMMddhhmm"));
        if (dt.date().year() < 2000) {
            dt = dt.addYears(100);
        }
        dt.setTimeSpec(Qt::UTC);
        return dt;
    }

    return {};
}

Person Uic9183Parser::person() const
{
    // Deutsche Bahn vendor block
    const auto b = findBlock<Vendor0080BLBlock>();
    if (b.isValid()) {
        // S028 contains family and given name separated by a '#', UTF-8 encoded
        auto sblock = b.findSubBlock("028");
        if (!sblock.isNull()) {
            const auto endIt = sblock.content() + sblock.contentSize();
            auto it = std::find(sblock.content(), endIt, '#');
            if (it != endIt) {
                Person p;
                p.setGivenName(QString::fromUtf8(sblock.content(), std::distance(sblock.content(), it)));
                ++it;
                p.setFamilyName(QString::fromUtf8(it, std::distance(it, endIt)));
                return p;
            }
        }
        // S023 contains the full name, UTF-8 encoded
        sblock = b.findSubBlock("023");
        if (!sblock.isNull()) {
            Person p;
            p.setName(sblock.toString());
            return p;
        }
    }

    // RCT2 tickets
    const auto rct2 = rct2Ticket();
    if (rct2.isValid()) {
        const auto name = rct2.passengerName();
        if (!name.isEmpty()) {
            Person p;
            p.setName(name);
            return p;
        }
    }

    return {};
}

QString Uic9183Parser::outboundDepartureStationId() const
{
    const auto b = findBlock<Vendor0080BLBlock>();
    if (b.isValid()) {
        // S035 contains the IBNR, possible with leading '80' country code and leading 0 stripped
        const auto sblock = b.findSubBlock("035");
        if (!sblock.isNull() && sblock.contentSize() <= 7) {
            QString ibnr = QStringLiteral("ibnr:8000000");
            const auto s = sblock.toString();
            return ibnr.replace(ibnr.size() - s.size(), s.size(), s);
        }
    }
    return {};
}

QString Uic9183Parser::outboundArrivalStationId() const
{
    const auto b = findBlock<Vendor0080BLBlock>();
    if (b.isValid()) {
        // S036 contains the IBNR, possible with leading '80' country code and leading 0 stripped
        const auto sblock = b.findSubBlock("036");
        if (!sblock.isNull() && sblock.contentSize() <= 7) {
            QString ibnr = QStringLiteral("ibnr:8000000");
            const auto s = sblock.toString();
            return ibnr.replace(ibnr.size() - s.size(), s.size(), s);
        }
    }
    return {};
}

QString Uic9183Parser::seatingType() const
{
    const auto b = findBlock<Vendor0080BLBlock>();;
    if (b.isValid()) {
        // S014 contains the class, possibly with a leading 'S' for some reason
        const auto sblock = b.findSubBlock("014");
        if (!sblock.isNull()) {
            const auto s = sblock.toString();
            return s.startsWith(QLatin1Char('S')) ? s.right(1) : s;
        }
    }

    const auto rct2 = rct2Ticket();
    if (rct2.isValid()) {
        return rct2.outboundClass();
    }
    return {};
}

Uic9183TicketLayout Uic9183Parser::ticketLayout() const
{
    return findBlock<Uic9183TicketLayout>();
}

QVariant Uic9183Parser::ticketLayoutVariant() const
{
    const auto layout = ticketLayout();
    return layout.isValid() ? QVariant::fromValue(layout) : QVariant();
}

Rct2Ticket Uic9183Parser::rct2Ticket() const
{
    Rct2Ticket rct2(ticketLayout());
    const auto u_head = findBlock<Uic9183Head>();
    rct2.setContextDate(u_head.issuingDateTime());
    return rct2;
}

QVariant Uic9183Parser::rct2TicketVariant() const
{
    const auto rct2 = rct2Ticket();
    if (rct2.isValid()) {
        return QVariant::fromValue(rct2);
    }
    return {};
}

Uic9183Header Uic9183Parser::header() const
{
    return Uic9183Header(d->m_data);
}

QByteArray Uic9183Parser::rawData() const
{
    return d->m_data;
}

bool Uic9183Parser::maybeUic9183(const QByteArray& data)
{
    Uic9183Header h(data);
    return h.isValid();
}

#include "moc_uic9183parser.cpp"
