/*
    SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "fcbextractor_p.h"

#include "variantvisitor_p.h"

#include <KItinerary/ExtractorValidator>
#include <KItinerary/Organization>
#include <KItinerary/Person>
#include <KItinerary/ProgramMembership>
#include <KItinerary/Reservation>
#include <KItinerary/Ticket>
#include <KItinerary/TrainTrip>

#include <type_traits>

using namespace Qt::Literals;
using namespace KItinerary;

template <typename T>
[[nodiscard]] static QString ticketNameForDocument(const T &doc)
{
    return std::visit([](auto &&doc) {
        if constexpr (is_any_of_v<decltype(doc), FCB_VERSIONED(ReservationData), FCB_VERSIONED(OpenTicketData), FCB_VERSIONED(PassData)>) {
            auto n = doc.tariffs.isEmpty() ? QString() : doc.tariffs.at(0).tariffDesc;
            if (!n.isEmpty()) {
                return n;
            }
            if constexpr (is_any_of_v<decltype(doc), FCB_VERSIONED(PassData)>) {
                if (!doc.passDescription.isEmpty()) {
                    return doc.passDescription;
                }
            }
            return doc.infoText;
        }
        return QString();
    }, doc);
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
    if constexpr (is_any_of_v<decltype(data), FCB_VERSIONED(ReservationData), FCB_VERSIONED(OpenTicketData), FCB_VERSIONED(PassData)>) {
        if (!data.referenceIA5.isEmpty()) {
            return QString::fromLatin1(data.referenceIA5);
        }
        if (data.referenceNumIsSet()) {
            return QString::number(data.referenceNum);
        }
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
            auto pnr = std::visit([](auto &&doc) {
                return fcbReference(doc);
            }, doc.ticket);
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
            auto s = std::visit([](auto &&doc) {
                if constexpr (is_any_of_v<decltype(doc), FCB_VERSIONED(ReservationData), FCB_VERSIONED(OpenTicketData), FCB_VERSIONED(PassData)>) {
                    return FcbUtil::classCodeToString(doc.classCode);
                }
                return QString{};
            }, doc.ticket);
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

QDateTime FcbExtractor::issuingDateTime(const Fcb::UicRailTicketData &fcb)
{
    return std::visit([](auto &&data) { return data.issuingDetail.issueingDateTime(); }, fcb);
}

QDateTime FcbExtractor::validFrom(const Fcb::UicRailTicketData &fcb)
{
    return std::visit([](auto &&fcb) {
        for (const auto &doc : fcb.transportDocument) {
            auto dt = std::visit([&fcb](auto &&doc) {
                if constexpr (is_any_of_v<decltype(doc), FCB_VERSIONED(ReservationData)>) {
                    return doc.departureDateTime(fcb.issuingDetail.issueingDateTime());
                }
                return QDateTime();
            }, doc.ticket);
            if (dt.isValid()) {
                return dt;
            }
            dt = std::visit([&fcb](auto &&doc) {
                if constexpr (is_any_of_v<decltype(doc), FCB_VERSIONED(OpenTicketData), FCB_VERSIONED(PassData)>) {
                    return doc.validFrom(fcb.issuingDetail.issueingDateTime());
                }
                return QDateTime();
            }, doc.ticket);
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
            auto dt = std::visit([&fcb](auto &&doc) {
                if constexpr (is_any_of_v<decltype(doc), FCB_VERSIONED(ReservationData)>) {
                    return doc.arrivalDateTime(fcb.issuingDetail.issueingDateTime());
                }
                return QDateTime();
            }, doc.ticket);
            if (dt.isValid()) {
                return dt;
            }
            dt = std::visit([&fcb](auto &&doc) {
                if constexpr (is_any_of_v<decltype(doc), FCB_VERSIONED(OpenTicketData), FCB_VERSIONED(PassData)>) {
                    return doc.validUntil(fcb.issuingDetail.issueingDateTime());
                }
                return QDateTime();
            }, doc.ticket);
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
            p.price = std::visit([fract](auto &&doc) -> double {
                if constexpr (is_any_of_v<decltype(doc), FCB_VERSIONED(ReservationData), FCB_VERSIONED(OpenTicketData), FCB_VERSIONED(PassData)>) {
                    return doc.priceIsSet() ? doc.price / fract : NAN;
                }
                return NAN;
            }, doc.ticket);
            if (!std::isnan(p.price)) {
                continue;
            }
        }
        return p;
    }, fcb);
}

template <typename CardReferenceTypeT>
static ProgramMembership extractCustomerCard(const CardReferenceTypeT &card)
{
    ProgramMembership p;
    p.setProgramName(card.cardName);
    if (card.cardIdNumIsSet()) {
        p.setMembershipNumber(QString::number(card.cardIdNum));
    } else if (card.cardIdIA5IsSet()) {
        p.setMembershipNumber(QString::fromUtf8(card.cardIdIA5));
    }
    return p;
}

template <typename TariffTypeT>
static ProgramMembership extractCustomerCard(const QList <TariffTypeT> &tariffs)
{
    // TODO what do we do with the (so far theoretical) case of multiple discount cards in use?
    for (const auto &tariff : tariffs) {
        for (const auto &card : tariff.reductionCard) {
            return extractCustomerCard(card);
        }
    }

    return {};
}

void FcbExtractor::extractReservation(const QVariant &res, const Fcb::UicRailTicketData &fcb, const Ticket &ticket, QList<QVariant> &result)
{
    const auto issuingDateTime = FcbExtractor::issuingDateTime(fcb);
    VariantVisitor([&fcb, &result, ticket, issuingDateTime](auto &&irt) {
        Ticket t(ticket);

        TrainTrip trip;
        trip.setProvider(FcbExtractor::issuer(fcb));
        if (trip.provider().identifier().isEmpty() && trip.provider().name().isEmpty()) {
            trip.setProvider(ticket.issuedBy());
        }
        t.setIssuedBy({});

        TrainStation dep;
        FcbExtractor::readDepartureStation(irt, dep);
        trip.setDepartureStation(dep);

        TrainStation arr;
        FcbExtractor::readArrivalStation(irt, arr);
        trip.setArrivalStation(arr);

        trip.setDepartureTime(irt.departureDateTime(issuingDateTime));
        trip.setArrivalTime(irt.arrivalDateTime(issuingDateTime));

        if (irt.trainNumIsSet()) {
            trip.setTrainNumber(irt.serviceBrandAbrUTF8 + ' '_L1 + QString::number(irt.trainNum));
        } else {
            trip.setTrainNumber(irt.serviceBrandAbrUTF8 + ' '_L1 + QString::fromUtf8(irt.trainIA5));
        }

        Seat s;
        s.setSeatingType(FcbUtil::classCodeToString(irt.classCode));
        if (irt.placesIsSet()) {
            s.setSeatSection(QString::fromUtf8(irt.places.coach));
            QStringList l;
            for (const auto &b : irt.places.placeIA5) {
                l.push_back(QString::fromUtf8(b));
            }
            for (auto i : irt.places.placeNum) {
                l.push_back(QString::number(i));
            }
            s.setSeatNumber(l.join(", "_L1));
            // TODO other seat encoding variants
        }
        t.setTicketedSeat(s);

        TrainReservation res;
        res.setReservationNumber(FcbExtractor::pnr(fcb));
        if (res.reservationNumber().isEmpty()) {
            res.setReservationNumber(ticket.ticketNumber());
        }
        t.setTicketNumber(fcbReference(irt));
        res.setUnderName(FcbExtractor::person(fcb));
        res.setProgramMembershipUsed(::extractCustomerCard(irt.tariffs));

        if (irt.priceIsSet()) {
            res.setTotalPrice(irt.price / std::pow(10, std::visit([](auto &&fcb) { return fcb.issuingDetail.currencyFract; }, fcb)));
        }
        res.setPriceCurrency(QString::fromUtf8(std::visit([](auto &&fcb) { return fcb.issuingDetail.currency; }, fcb)));

        ExtractorValidator validator;
        validator.setAcceptedTypes<TrainTrip>();
        if (validator.isValidElement(trip)) {
            res.setReservationFor(trip);
            res.setReservedTicket(t);
            result.push_back(res);
        }
    }).visit<FCB_VERSIONED(ReservationData)>(res);
}

template <typename T, typename CodeTableType>
[[nodiscard]] static bool extractValidRegion(const T &regionalValidity, CodeTableType stationCodeTable, const QDateTime &issuingDateTime, const TrainReservation &baseRes, const TrainTrip &baseTrip, QList<QVariant> &result)
{
    return std::visit([&baseTrip, stationCodeTable, issuingDateTime, &baseRes, &result](auto &&trainLink) {
        if constexpr (is_any_of_v<decltype(trainLink), FCB_VERSIONED(TrainLinkType)>) {
            TrainTrip trip(baseTrip);

            // TODO station identifier, use FcbExtractor::read[Arrival|Departure]Station
            if (trainLink.fromStationNameUTF8IsSet()) {
                TrainStation dep;
                FcbExtractor::readDepartureStation(trainLink, stationCodeTable, dep);
                trip.setDepartureStation(dep);
            }

            if (trainLink.toStationNameUTF8IsSet()) {
                TrainStation arr;
                FcbExtractor::readArrivalStation(trainLink, stationCodeTable, arr);
                trip.setArrivalStation(arr);
            }

            trip.setDepartureDay({}); // reset explicit value in case of departure after midnight
            trip.setDepartureTime(trainLink.departureDateTime(issuingDateTime));

            if (trainLink.trainNumIsSet()) {
                trip.setTrainNumber(QString::number(trainLink.trainNum));
            } else {
                trip.setTrainNumber(QString::fromUtf8(trainLink.trainIA5));
            }

            ExtractorValidator validator;
            validator.setAcceptedTypes<TrainTrip>();
            if (validator.isValidElement(trip)) {
                TrainReservation res(baseRes);
                res.setReservationFor(trip);
                result.push_back(res);
                return true;
            }
        }

        return false;
    }, regionalValidity);
}

void FcbExtractor::extractOpenTicket(const QVariant &res, const Fcb::UicRailTicketData &fcb, const Ticket &ticket, QList<QVariant> &result)
{
    const auto issuingDateTime = FcbExtractor::issuingDateTime(fcb);
    VariantVisitor([&fcb, ticket, &result, issuingDateTime] (auto &&nrt) {
        Seat s;
        s.setSeatingType(FcbUtil::classCodeToString(nrt.classCode));
        Ticket t(ticket);
        t.setTicketedSeat(s);

        TrainReservation res;
        res.setReservationNumber(FcbExtractor::pnr(fcb));
        if (res.reservationNumber().isEmpty()) {
            res.setReservationNumber(ticket.ticketNumber());
        }
        t.setTicketNumber(fcbReference(nrt));
        t.setIssuedBy({});
        res.setReservedTicket(t);

        res.setUnderName(FcbExtractor::person(fcb));
        res.setProgramMembershipUsed(::extractCustomerCard(nrt.tariffs));

        if (nrt.priceIsSet()) {
            res.setTotalPrice(nrt.price / std::pow(10, std::visit([](auto &&fcb) { return fcb.issuingDetail.currencyFract; }, fcb)));
        }
        res.setPriceCurrency(QString::fromUtf8(std::visit([](auto &&fcb) { return fcb.issuingDetail.currency; }, fcb)));

        TrainTrip baseTrip;
        baseTrip.setProvider(FcbExtractor::issuer(fcb));
        if (baseTrip.provider().name().isEmpty() && baseTrip.provider().identifier().isEmpty()) {
            baseTrip.setProvider(ticket.issuedBy());
        }
        TrainStation dep;
        FcbExtractor::readDepartureStation(nrt, dep);
        baseTrip.setDepartureStation(dep);
        TrainStation arr;
        FcbExtractor::readArrivalStation(nrt, arr);
        baseTrip.setArrivalStation(arr);
        baseTrip.setDepartureDay(nrt.validFrom(issuingDateTime).date());

        ExtractorValidator validator;
        validator.setAcceptedTypes<TrainTrip>();

        // check for TrainLinkType regional validity constrains
        bool trainLinkTypeFound = false;
        for (const auto &regionalValidity : nrt.validRegion) {
            trainLinkTypeFound |= extractValidRegion(regionalValidity.value, nrt.stationCodeTable, issuingDateTime, res, baseTrip, result);
        }

        if (!trainLinkTypeFound) {
            if (validator.isValidElement(baseTrip)) {
                res.setReservationFor(baseTrip);
                result.push_back(res);
            }
        }

        // same for return trips
        if (nrt.returnIncluded) {
            TrainStation retDep;
            FcbExtractor::readDepartureStation(nrt.returnDescription, nrt.stationCodeTable, retDep);
            TrainStation retArr;
            FcbExtractor::readArrivalStation(nrt.returnDescription, nrt.stationCodeTable, retArr);

            TrainTrip retBaseTrip;
            retBaseTrip.setProvider(baseTrip.provider());
            retBaseTrip.setDepartureStation(retDep);
            retBaseTrip.setArrivalStation(retArr);

            bool retTrainLinkTypeFound = false;
            for (const auto &regionalValidity : nrt.returnDescription.validReturnRegion) {
                retTrainLinkTypeFound |= extractValidRegion(regionalValidity.value, nrt.stationCodeTable, issuingDateTime, res, retBaseTrip, result);
            }

            if (!retTrainLinkTypeFound && validator.isValidElement(retBaseTrip)) {
                res.setReservationFor(retBaseTrip);
                result.push_back(retBaseTrip);
            }
        }
    }).visit<FCB_VERSIONED(OpenTicketData)>(res);
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
    }).visit<FCB_VERSIONED(CustomerCardData)>(ccd);
}

void FcbExtractor::readDepartureStation(const QVariant &doc, TrainStation &station)
{
    VariantVisitor([&station](auto &&data) {
        FcbExtractor::readDepartureStation(data, station);
    }).visit<FCB_VERSIONED(ReservationData), FCB_VERSIONED(OpenTicketData)>(doc);
}

void FcbExtractor::readArrivalStation(const QVariant &doc, TrainStation &station)
{
    VariantVisitor([&station](auto &&data) {
        FcbExtractor::readArrivalStation(data, station);
    }).visit<FCB_VERSIONED(ReservationData), FCB_VERSIONED(OpenTicketData)>(doc);
}

void FcbExtractor::fixStationCode(TrainStation &station)
{
    // UIC codes in Germany are wildly unreliable, there seem to be different
    // code tables in use by different operators, so we unfortunately have to ignore
    // those entirely
    if (station.identifier().startsWith("uic:80"_L1)) {
      PostalAddress addr;
      addr.setAddressCountry(u"DE"_s);
      station.setAddress(addr);
      station.setIdentifier(QString());
    }
}
