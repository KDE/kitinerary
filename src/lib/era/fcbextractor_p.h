/*
    SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KITINERARY_FCBEXTRACTOR_H
#define KITINERARY_FCBEXTRACTOR_H

#include "era/fcbticket.h"
#include "era/fcbutil.h"

#include <KItinerary/Place>

#include <QVariant>

class QString;

namespace KItinerary {

class Organization;
class Person;
class Ticket;
class TrainStation;

/** Generic extractor functions for FCB tickets. */
class FcbExtractor
{
public:
    /** Human-readable name of the ticket, typically what would be printed on top. */
    [[nodiscard]] static QString ticketName(const Fcb::UicRailTicketData &fcb);

    /** Reference number/PNR. */
    [[nodiscard]] static QString pnr(const Fcb::UicRailTicketData &fcb);

    /** Seating class, if available. */
    [[nodiscard]] static QString seatingType(const Fcb::UicRailTicketData &fcb);

    /** Issuer organization of this ticket. */
    [[nodiscard]] static QString issuerId(const Fcb::UicRailTicketData &fcb);
    [[nodiscard]] static Organization issuer(const Fcb::UicRailTicketData &fcb);
    /** Traveller who this ticket is for. */
    [[nodiscard]] static Person person(const Fcb::UicRailTicketData &fcb);

    /** Issuing date/time. */
    [[nodiscard]] static QDateTime issuingDateTime(const Fcb::UicRailTicketData &fcb);
    /** Ticket validity range. */
    [[nodiscard]] static QDateTime validFrom(const Fcb::UicRailTicketData &fcb);
    [[nodiscard]] static QDateTime validUntil(const Fcb::UicRailTicketData &fcb);

    /** Price/currency. */
    template <typename T>
    static inline void applyPrice(T &obj, const Fcb::UicRailTicketData &fcb)
    {
        const auto p = FcbExtractor::price(fcb);
        if (!std::isnan(p.price)) {
            obj.setPriceCurrency(p.currency);
            obj.setTotalPrice(p.price);
        }
    }

    /** Extract reservation documents. */
    static void extractReservation(const QVariant &res,  const Fcb::UicRailTicketData &fcb, const Ticket &ticket, QList<QVariant> &result);
    static void extractOpenTicket(const QVariant &res,  const Fcb::UicRailTicketData &fcb, const Ticket &ticket, QList<QVariant> &result);
    /** Extract from transport documents. */
    static void extractCustomerCard(const QVariant &ccd, const Fcb::UicRailTicketData &fcb, const Ticket &ticket, QList<QVariant> &result);

    /** Read departure station info from the given FCB travel document, if applicable. */
    static void readDepartureStation(const QVariant &doc, TrainStation &station);
    template <typename T, typename StationCodeTable>
    static inline void readDepartureStation(const T &data, StationCodeTable codeTab, TrainStation &station)
    {
        station.setName(data.fromStationNameUTF8);
        station.setIdentifier(FcbUtil::fromStationIdentifier(codeTab, data));
        fixStationCode(station);
    }
    template <typename T>
    static inline void readDepartureStation(const T &data, TrainStation &station)
    {
        readDepartureStation(data, data.stationCodeTable, station);
    }
    /** Read arrival station info from the given FCB travel document, if applicable. */
    static void readArrivalStation(const QVariant &doc, TrainStation &station);
    template <typename T, typename StationCodeTable>
    static inline void readArrivalStation(const T &data, StationCodeTable codeTab, TrainStation &station)
    {
        station.setName(data.toStationNameUTF8);
        station.setIdentifier(FcbUtil::toStationIdentifier(codeTab, data));
        fixStationCode(station);
    }
    template <typename T>
    static inline void readArrivalStation(const T &data, TrainStation &station)
    {
        readArrivalStation(data, data.stationCodeTable, station);
    }
    /** Fix known issues with station identifiers. */
    static void fixStationCode(TrainStation &station);

private:
    struct PriceData {
        double price = NAN;
        QString currency;
    };
    [[nodiscard]] static PriceData price(const Fcb::UicRailTicketData &fcb);
};

}

#endif
