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
    static QString fromStationIdentifier(Fcb::CodeTableType stationCodeTable, const T &doc)
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
    static QString fromStationIdentifier(const T &doc) { return fromStationIdentifier(doc.stationCodeTable, doc); }
    /** Arrival station identifier for a travel document,
    *  in the format needed for output with our JSON-LD format.
    */
    template <typename T>
    static QString toStationIdentifier(Fcb::CodeTableType stationCodeTable, const T &doc)
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
    static QString toStationIdentifier(const T &doc) { return toStationIdentifier(doc.stationCodeTable, doc); }

private:
    static QString stringifyUicStationIdentifier(int num, const QByteArray &ia5);
    static QString stringifyStationIdentifier(bool numIsSet, int num, const QByteArray ia5);
};

}

#endif // KITINERARY_FCBUTIL_H