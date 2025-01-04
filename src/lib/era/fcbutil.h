/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KITINERARY_FCBUTIL_H
#define KITINERARY_FCBUTIL_H

#include "fcbticket.h"
#include "logging.h"

#include <QString>

namespace KItinerary {

/** Higher-level decoding utilities for ERA FCB ticket data. */
class FcbUtil
{
public:
    /** Departure station identifier for a travel document,
    *  in the format needed for output with our JSON-LD format.
    */
    template <typename T>
    [[nodiscard]] static QString fromStationIdentifier(Fcb::CodeTableType stationCodeTable, const T &doc)
    {
        switch (stationCodeTable) {
            case Fcb::stationUIC:
            case Fcb::stationUICReservation:
                return stringifyUicStationIdentifier(doc.fromStationNum, doc.fromStationIA5);
            default:
                qCWarning(Log) << "Unhandled station code table:" << stationCodeTable;
        }
        return stringifyStationIdentifier(doc.fromStationNumIsSet(), doc.fromStationNum, doc.fromStationIA5);
    }
    template <typename T>
    [[nodiscard]] static QString fromStationIdentifier(const T &doc) { return fromStationIdentifier(doc.stationCodeTable, doc); }
    /** Arrival station identifier for a travel document,
    *  in the format needed for output with our JSON-LD format.
    */
    template <typename T>
    [[nodiscard]] static QString toStationIdentifier(Fcb::CodeTableType stationCodeTable, const T &doc)
    {
        switch (stationCodeTable) {
            case Fcb::stationUIC:
            case Fcb::stationUICReservation:
                return stringifyUicStationIdentifier(doc.toStationNum, doc.toStationIA5);
            default:
                qCWarning(Log) << "Unhandled station code table:" << stationCodeTable;
        }
        return stringifyStationIdentifier(doc.toStationNumIsSet(), doc.toStationNum, doc.toStationIA5);
    }
    template <typename T>
    [[nodiscard]] static QString toStationIdentifier(const T &doc) { return toStationIdentifier(doc.stationCodeTable, doc); }

    /** Convert a class code enum value to a string for human representation. */
    [[nodiscard]] static QString classCodeToString(Fcb::TravelClassType classCode);

    /** Decode FCB date. */
    [[nodiscard]] static QDate decodeDate(int year, std::optional<int> day);
    [[nodiscard]] static QDate decodeDifferentialDate(const QDate &base, int year, std::optional<int> day);

    /** Decode FCB issuing time. */
    [[nodiscard]] static QDateTime issuingDateTime(int year, int day, std::optional<int> time);
    /** Decode differential time, relative to @p baseDt. */
    [[nodiscard]] static QDateTime decodeDifferentialTime(const QDateTime &baseDt, int day, std::optional<int> time, std::optional<int> utcOffset);
    [[nodiscard]] static QDateTime decodeDifferentialStartTime(const QDateTime &baseDt, int day, std::optional<int> time, std::optional<int> utcOffset);
    [[nodiscard]] static QDateTime decodeDifferentialEndTime(const QDateTime &baseDt, int day, std::optional<int> time, std::optional<int> utcOffset);

private:
    [[nodiscard]] static QString stringifyUicStationIdentifier(int num, const QByteArray &ia5);
    [[nodiscard]] static QString stringifyStationIdentifier(bool numIsSet, int num, const QByteArray &ia5);
};

}

#endif // KITINERARY_FCBUTIL_H
