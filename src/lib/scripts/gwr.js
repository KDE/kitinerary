/*
   SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
   SPDX-License-Identifier: LGPL-2.0-or-later
*/

function parseEvent(event) {
    let res = JsonLd.newTrainReservation();
    res.reservationFor.departureTime = JsonLd.readQDateTime(event, 'dtStart');
    res.reservationFor.arrivalTime = JsonLd.readQDateTime(event, 'dtEnd');
    const trip = event.description.match(/Journey Details: (.*) \(([A-Z]{3})\) to (.*) \(([A-Z]{3})\)/);
    res.reservationFor.departureStation.name = trip[1];
    res.reservationFor.departureStation.identifier = 'uk:' + trip[2];
    res.reservationFor.arrivalStation.name = trip[3];
    res.reservationFor.arrivalStation.identifier = 'uk:' + trip[4];
    return res;
}
