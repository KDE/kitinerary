/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KITINERARY_RESERVATIONCONVERTER_H
#define KITINERARY_RESERVATIONCONVERTER_H

class QJsonObject;

namespace KItinerary {

/** Convert between different types of reservations, to the extend possible. */
namespace ReservationConverter
{
    /** Convert a train reservation to a bus reservation. */
    [[nodiscard]] QJsonObject trainToBus(const QJsonObject &trainRes);
    /** Convert a bus reservation to a train reservation. */
    [[nodiscard]] QJsonObject busToTrain(const QJsonObject &busRes);
    /** Convert a flight reservation to a train reservation. */
    [[nodiscard]] QJsonObject flightToTrain(const QJsonObject&flightRes);

}

}

#endif
