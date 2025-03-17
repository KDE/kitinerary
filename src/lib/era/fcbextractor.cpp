/*
    SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "fcbextractor_p.h"
#include "fcbutil.h"

#include "variantvisitor_p.h"

#include <KItinerary/Organization>
#include <KItinerary/Person>
#include <KItinerary/ProgramMembership>
#include <KItinerary/Ticket>

#include <type_traits>

using namespace Qt::Literals;
using namespace KItinerary;

[[nodiscard]] static QString ticketNameForDocument(const QVariant &doc)
{
    return VariantVisitor([](auto &&doc) {
        auto n = doc.tariffs.isEmpty() ? QString() : doc.tariffs.at(0).tariffDesc;
        if (!n.isEmpty()) {
            return n;
        }
        if constexpr (std::is_same_v<std::decay_t<decltype(doc)>, Fcb::v13::PassData> || std::is_same_v<std::decay_t<decltype(doc)>, Fcb::v3::PassData>) {
            if (!doc.passDescription.isEmpty()) {
                return doc.passDescription;
            }
        }

        return doc.infoText;
    }).visit<Fcb::v13::ReservationData, Fcb::v13::OpenTicketData, Fcb::v13::PassData, Fcb::v3::ReservationData, Fcb::v3::OpenTicketData, Fcb::v3::PassData>(doc);
}

QString FcbExtractor::ticketName(const Fcb::UicRailTicketData &fcb)
{
    return std::visit([](auto &&fcb) {
        for (const auto &doc : fcb.transportDocument) {
            if (auto n = ticketNameForDocument(doc.ticket); !n.isEmpty()) {
                return n;
            }
        }
        return QString();
    }, fcb);
}

template <typename T>
[[nodiscard]] static QString fcbReference(const T &data)
{
    if (!data.referenceIA5.isEmpty()) {
        return QString::fromLatin1(data.referenceIA5);
    }
    if (data.referenceNumIsSet()) {
        return QString::number(data.referenceNum);
    }
    return {};
}

QString FcbExtractor::pnr(const Fcb::UicRailTicketData &fcb)
{
    return std::visit([](auto &&fcb) {
        if (!fcb.issuingDetail.issuerPNR.isEmpty()) {
            return QString::fromLatin1(fcb.issuingDetail.issuerPNR);
        }

        for (const auto &doc : fcb.transportDocument) {
            auto pnr = VariantVisitor([](auto &&doc) {
                return fcbReference(doc);
            }).template visit<Fcb::v13::ReservationData, Fcb::v13::OpenTicketData, Fcb::v13::PassData, Fcb::v3::ReservationData, Fcb::v3::OpenTicketData, Fcb::v3::PassData>(doc.ticket);
            if (!pnr.isEmpty()) {
                return pnr;
            }
        }

        return QString();
    }, fcb);
}

QString FcbExtractor::seatingType(const Fcb::UicRailTicketData &fcb)
{
    return std::visit([](auto &&fcb) {
        for (const auto &doc : fcb.transportDocument) {
            auto s = VariantVisitor([](auto &&doc) {
                return FcbUtil::classCodeToString(doc.classCode);
            }).template visit<Fcb::v13::ReservationData, Fcb::v13::OpenTicketData, Fcb::v13::PassData, Fcb::v3::ReservationData, Fcb::v3::OpenTicketData, Fcb::v3::PassData>(doc.ticket);
            if (!s.isEmpty()) {
                return s;
            }
        }
        return QString();
    }, fcb);
}

[[nodiscard]] static QString formatIssuerId(int num)
{
    auto id = QString::number(num);
    if (id.size() < 4) {
        id.insert(0, QString(4 - id.size(), '0'_L1));
    }
    return id;
}

QString FcbExtractor::issuerId(const Fcb::UicRailTicketData &fcb)
{
    return std::visit([](auto &&fcb) {
        if (fcb.issuingDetail.issuerNumIsSet()) {
            return formatIssuerId(fcb.issuingDetail.issuerNum);
        }
        if (fcb.issuingDetail.issuerIA5IsSet()) {
            return QString::fromLatin1(fcb.issuingDetail.issuerIA5);
        }
        if (fcb.issuingDetail.securityProviderNumIsSet()) {
            return formatIssuerId(fcb.issuingDetail.securityProviderNum);
        }
        if (fcb.issuingDetail.securityProviderIA5IsSet()) {
            return QString::fromLatin1(fcb.issuingDetail.securityProviderIA5);
        }
        return QString();
    }, fcb);
}

Organization FcbExtractor::issuer(const Fcb::UicRailTicketData &fcb)
{
    Organization issuer;
    if (auto id = issuerId(fcb); !id.isEmpty()) {
        issuer.setIdentifier("uic:"_L1 + id);
    }
    std::visit([&issuer](auto &&fcb) {
        if (fcb.issuingDetail.issuerNameIsSet()) {
            issuer.setName(fcb.issuingDetail.issuerName);
        }
    }, fcb);
    return issuer;
}

Person FcbExtractor::person(const Fcb::UicRailTicketData &fcb)
{
    return std::visit([](auto &&fcb) {
        Person p;
        if (!fcb.travelerDetailIsSet() || fcb.travelerDetail.traveler.size() != 1) {
            return p;
        }
        const auto traveler = fcb.travelerDetail.traveler.at(0);
        if (traveler.firstNameIsSet() || traveler.secondNameIsSet()) {
            p.setGivenName(QString(traveler.firstName + ' '_L1 + traveler.secondName).trimmed());
        }
        p.setFamilyName(traveler.lastName);
        return p;
    }, fcb);
}

QDateTime FcbExtractor::validFrom(const Fcb::UicRailTicketData &fcb)
{
    return std::visit([](auto &&fcb) {
        for (const auto &doc : fcb.transportDocument) {
            auto dt = VariantVisitor([&fcb](auto &&doc) {
                return doc.departureDateTime(fcb.issuingDetail.issueingDateTime());
            }).template visit<Fcb::v13::ReservationData, Fcb::v3::ReservationData>(doc.ticket);
            if (dt.isValid()) {
                return dt;
            }
            dt = VariantVisitor([&fcb](auto &&doc) {
                return doc.validFrom(fcb.issuingDetail.issueingDateTime());
            }).template visit<Fcb::v13::OpenTicketData, Fcb::v13::PassData, Fcb::v3::OpenTicketData, Fcb::v3::PassData>(doc.ticket);
            if (dt.isValid()) {
                return dt;
            }
        }
        return QDateTime();
    }, fcb);
}

QDateTime FcbExtractor::validUntil(const Fcb::UicRailTicketData &fcb)
{
    return std::visit([](auto &&fcb) {
        for (const auto &doc : fcb.transportDocument) {
            auto dt = VariantVisitor([&fcb](auto &&doc) {
                return doc.arrivalDateTime(fcb.issuingDetail.issueingDateTime());
            }).template visit<Fcb::v13::ReservationData, Fcb::v3::ReservationData>(doc.ticket);
            if (dt.isValid()) {
                return dt;
            }
            dt = VariantVisitor([&fcb](auto &&doc) {
                return doc.validUntil(fcb.issuingDetail.issueingDateTime());
            }).template visit<Fcb::v13::OpenTicketData, Fcb::v13::PassData, Fcb::v3::OpenTicketData, Fcb::v3::PassData>(doc.ticket);
            if (dt.isValid()) {
                return dt;
            }
        }
        return QDateTime();
    }, fcb);
}

FcbExtractor::PriceData FcbExtractor::price(const Fcb::UicRailTicketData &fcb)
{
    return std::visit([](auto &&fcb) {
        PriceData p;
        p.currency = QString::fromUtf8(fcb.issuingDetail.currency);
        const auto fract = std::pow(10, fcb.issuingDetail.currencyFract);
        for (const auto &doc : fcb.transportDocument) {
            p.price = VariantVisitor([fract](auto &&doc) {
                return doc.priceIsSet() ? doc.price / fract : NAN;
            }).template visit<Fcb::v13::ReservationData, Fcb::v3::ReservationData, Fcb::v13::OpenTicketData, Fcb::v13::PassData, Fcb::v3::OpenTicketData, Fcb::v13::PassData, Fcb::v3::PassData>(doc.ticket);
            if (!std::isnan(p.price)) {
                continue;
            }
        }
        return p;
    }, fcb);
}

void FcbExtractor::extractCustomerCard(const QVariant &ccd, const Fcb::UicRailTicketData &fcb, const Ticket &ticket, QList<QVariant> &result)
{
    VariantVisitor([&fcb, &result, ticket](auto &&ccd) {
        ProgramMembership pm;
        if (ccd.cardIdNumIsSet()) {
            pm.setMembershipNumber(QString::number(ccd.cardIdNum));
        } else {
            pm.setMembershipNumber(QString::fromUtf8(ccd.cardIdIA5));
        }
        pm.setProgramName(ccd.cardTypeDescr);
        pm.setMember(FcbExtractor::person(fcb));
        pm.setValidFrom(ccd.validFrom().startOfDay());
        pm.setValidUntil(ccd.validUntil().startOfDay());
        pm.setToken(ticket.ticketToken());
        result.push_back(pm);
    }).visit<Fcb::v13::CustomerCardData, Fcb::v3::CustomerCardData>(ccd);
}
