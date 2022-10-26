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

#include "era/fcbticket.h"

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

#define BLOCK_FROM_NAME(Type) \
    if (name == QLatin1String(Type::RecordId)) { \
        const auto block = findBlock<Type>(); \
        return block.isValid() ? QVariant::fromValue(block) : QVariant(); \
    }

    BLOCK_FROM_NAME(Uic9183Head)
    BLOCK_FROM_NAME(Uic9183TicketLayout)
    BLOCK_FROM_NAME(Fcb::UicRailTicketData)
    BLOCK_FROM_NAME(Vendor0080BLBlock)
    BLOCK_FROM_NAME(Vendor0080VUBlock)

#undef BLOCK_FROM_NAME

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

template <typename T>
static QString fcbReference(const T &data)
{
    if (!data.referenceIA5.isEmpty()) {
        return QString::fromLatin1(data.referenceIA5);
    }
    if (data.referenceNumIsSet()) {
        return QString::number(data.referenceNum);
    }
    return {};
}

QString Uic9183Parser::pnr() const
{
    if (const auto head = findBlock<Uic9183Head>(); head.isValid()) {
        const auto key = head.ticketKey().trimmed();
        const auto issuerId = head.issuerCompanyCodeNumeric();

        // try to make this match what's printed on the matching tickets...
        if (issuerId == 80 && (key.size() == 8 || key.size() == 9) && key.at(6) == QLatin1Char('-') && key.at(7).isDigit()) {
            return key.left(6); // DB domestic
        }
        if (issuerId == 80 && key.size() == 13 && key.endsWith(QLatin1String("0101"))) {
            return key.left(9); // DB domestic part of an international order
        }
        if (issuerId == 1184 && key.size() == 9 && key.at(7) == QLatin1Char('_') && key.at(8).isDigit()) {
            return key.left(7); // NS
        }

        return key;
    }

    if (const auto fcb = findBlock<Fcb::UicRailTicketData>(); fcb.isValid()) {
        if (!fcb.issuingDetail.issuerPNR.isEmpty()) {
            return QString::fromLatin1(fcb.issuingDetail.issuerPNR);
        }
        if (!fcb.transportDocument.isEmpty()) {
            const auto doc = fcb.transportDocument.at(0);
            QString pnr;
            if (doc.ticket.userType() == qMetaTypeId<Fcb::ReservationData>()) {
                pnr = fcbReference(doc.ticket.value<Fcb::ReservationData>());
            } else if (doc.ticket.userType() == qMetaTypeId<Fcb::OpenTicketData>()) {
                pnr = fcbReference(doc.ticket.value<Fcb::OpenTicketData>());
            } else if (doc.ticket.userType() == qMetaTypeId<Fcb::PassData>()) {
                pnr = fcbReference(doc.ticket.value<Fcb::PassData>());
            }
            if (!pnr.isEmpty()) {
                return pnr;
            }
        }
    }

    return {};
}

template <typename T>
static QString fcbTariffName(const T &data)
{
    if (data.tariffs.isEmpty()) {
        return {};
    }
    return data.tariffs.at(0).tariffDesc;
}

QString Uic9183Parser::name() const
{
    // ERA FCB
    if (const auto fcb = findBlock<Fcb::UicRailTicketData>(); fcb.isValid() && !fcb.transportDocument.isEmpty()) {
        const auto doc = fcb.transportDocument.at(0);
        QString name;
        if (doc.ticket.userType() == qMetaTypeId<Fcb::ReservationData>()) {
            name = fcbTariffName(doc.ticket.value<Fcb::ReservationData>());
        } else if (doc.ticket.userType() == qMetaTypeId<Fcb::OpenTicketData>()) {
            name = fcbTariffName(doc.ticket.value<Fcb::OpenTicketData>());
        } else if (doc.ticket.userType() == qMetaTypeId<Fcb::PassData>()) {
            name = fcbTariffName(doc.ticket.value<Fcb::PassData>());
        }
        if (!name.isEmpty()) {
            return name;
        }
    }

    // DB vendor block
    if (const auto b = findBlock<Vendor0080BLBlock>(); b.isValid()) {
        const auto sblock = b.findSubBlock("001");
        if (!sblock.isNull()) {
            return QString::fromUtf8(sblock.content(), sblock.contentSize());
        }
    }

    // RCT2
    if (const auto rct2 = rct2Ticket(); rct2.isValid()) {
        return rct2.title();
    }

    return {};
}

QString Uic9183Parser::carrierId() const
{
    if (const auto head = findBlock<Uic9183Head>(); head.isValid()) {
        return head.issuerCompanyCodeString();
    }
    if (const auto fcb = findBlock<Fcb::UicRailTicketData>(); fcb.isValid()) {
        const auto issue = fcb.issuingDetail;
        if (issue.issuerNumIsSet()) {
            return QString::number(issue.issuerNum);
        }
        if (issue.issuerIA5IsSet()) {
            return QString::fromLatin1(issue.issuerIA5);
        }
    }
    return header().signerCompanyCode();
}

Organization Uic9183Parser::issuer() const
{
    Organization issuer;
    issuer.setIdentifier(QLatin1String("uic:") + carrierId());
    if (const auto fcb = findBlock<Fcb::UicRailTicketData>(); fcb.isValid() && fcb.issuingDetail.issuerNameIsSet()) {
        issuer.setName(fcb.issuingDetail.issuerName);
    }
    return issuer;
}

QDateTime Uic9183Parser::validFrom() const
{
    // ERA FCB
    if (const auto fcb = findBlock<Fcb::UicRailTicketData>(); fcb.isValid() && !fcb.transportDocument.isEmpty()) {
        const auto issue = fcb.issuingDetail.issueingDateTime();
        const auto doc = fcb.transportDocument.at(0).ticket;
        if (doc.userType() == qMetaTypeId<Fcb::ReservationData>()) {
            return doc.value<Fcb::ReservationData>().departureDateTime(issue);
        }
        if (doc.userType() == qMetaTypeId<Fcb::OpenTicketData>()) {
            return doc.value<Fcb::OpenTicketData>().validFrom(issue);
        }
        if (doc.userType() == qMetaTypeId<Fcb::PassData>()) {
            return doc.value<Fcb::PassData>().validFrom(issue);
        }
    }

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
        const auto dt = rct2.firstDayOfValidity();
        if (dt.month() != 1 || dt.day() != 1 || !rct2.outboundDepartureStation().isEmpty()) {
            return QDateTime(dt, {0, 0, 0});
        }
        // firstDayOfValidity is just a year, and we have wildcard station names
        const auto dep = rct2.outboundDepartureTime();
        return dep.isValid() ? dep : QDateTime(dt, {0, 0, 0});
    }

    return {};
}

QDateTime Uic9183Parser::validUntil() const
{
    // ERA FCB
    if (const auto fcb = findBlock<Fcb::UicRailTicketData>(); fcb.isValid() && !fcb.transportDocument.isEmpty()) {
        const auto issue = fcb.issuingDetail.issueingDateTime();
        const auto doc = fcb.transportDocument.at(0).ticket;
        if (doc.userType() == qMetaTypeId<Fcb::ReservationData>()) {
            return doc.value<Fcb::ReservationData>().arrivalDateTime(issue);
        }
        if (doc.userType() == qMetaTypeId<Fcb::OpenTicketData>()) {
            return doc.value<Fcb::OpenTicketData>().validUntil(issue);
        }
        if (doc.userType() == qMetaTypeId<Fcb::PassData>()) {
            return doc.value<Fcb::PassData>().validUntil(issue);
        }
    }

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

    // RCT2 RPT according to ERA TAP TSI Annex B.6
    if (const auto rct2 = rct2Ticket(); rct2.isValid() && rct2.type() == Rct2Ticket::RailPass) {
        const auto validityRange = ticketLayout().text(3, 1, 36, 1).trimmed();
        const auto idx = std::max(validityRange.lastIndexOf(QLatin1Char(' ')), validityRange.lastIndexOf(QLatin1Char('-')));
        if (idx > 0) {
            return QDateTime(QDate::fromString(validityRange.mid(idx + 1), QStringLiteral("dd.MM.yyyy")), {23, 59, 59});
        }
    }

    return {};
}

Person Uic9183Parser::person() const
{
    // ERA FCB
    if (const auto fcb = findBlock<Fcb::UicRailTicketData>(); fcb.isValid() && fcb.travelerDetailIsSet() && fcb.travelerDetail.traveler.size() == 1) {
        const auto traveler = fcb.travelerDetail.traveler.at(0);
        Person p;
        p.setGivenName(QString(traveler.firstName + QLatin1Char(' ') + traveler.secondName).trimmed());
        p.setFamilyName(traveler.lastName);
        if (traveler.firstNameIsSet() || traveler.lastNameIsSet()) {
            return p;
        }
    }

    // Deutsche Bahn vendor block
    if (const auto b = findBlock<Vendor0080BLBlock>(); b.isValid()) {
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

static QString fcbClassCodeToString(Fcb::TravelClassType classCode)
{
    switch (classCode) {
        case Fcb::notApplicable: return {};
        case Fcb::first: return QString::number(1);
        case Fcb::second: return QString::number(2);
        default:
            qCWarning(Log) << "Unhandled FCB class code" << classCode;
    }
    return {};
}

QString Uic9183Parser::seatingType() const
{
    if (const auto fcb = findBlock<Fcb::UicRailTicketData>(); fcb.isValid() && fcb.transportDocument.size() == 1) {
        const auto doc = fcb.transportDocument.at(0);
        if (doc.ticket.userType() == qMetaTypeId<Fcb::ReservationData>()) {
            return fcbClassCodeToString(doc.ticket.value<Fcb::ReservationData>().classCode);
        }
        if (doc.ticket.userType() == qMetaTypeId<Fcb::OpenTicketData>()) {
            return fcbClassCodeToString(doc.ticket.value<Fcb::OpenTicketData>().classCode);
        }
        if (doc.ticket.userType() == qMetaTypeId<Fcb::PassData>()) {
            return fcbClassCodeToString(doc.ticket.value<Fcb::PassData>().classCode);
        }
    }

    if (const auto b = findBlock<Vendor0080BLBlock>(); b.isValid()) {
        // S014 contains the class, possibly with a leading 'S' for some reason
        const auto sblock = b.findSubBlock("014");
        if (!sblock.isNull()) {
            const auto s = sblock.toString();
            return s.startsWith(QLatin1Char('S')) ? s.right(1) : s;
        }
    }

    if (const auto rct2 = rct2Ticket(); rct2.isValid()) {
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
