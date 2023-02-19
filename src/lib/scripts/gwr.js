/*
   SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
   SPDX-License-Identifier: LGPL-2.0-or-later
*/

function parseEvent(event) {
    let reservations = [];
    let idx = 0;
    const dtStart = JsonLd.readQDateTime(event, 'dtStart');
    while (true) {
        const trip = event.description.substr(idx).match(/Journey Details: (.*) \(([A-Z]{3})\) to (.*) \(([A-Z]{3})\), *dep *(\d\d):(\d\d), *arr *(\d\d):(\d\d)/);
        if (!trip) {
            break;
        }
        idx += trip.index + trip[0].length;

        let res = JsonLd.newTrainReservation();
        let depDt = new Date(dtStart);
        depDt.setHours(trip[5]);
        depDt.setMinutes(trip[6]);
        res.reservationFor.departureTime = depDt;
        let arrDt = new Date(dtStart);
        arrDt.setHours(trip[7]);
        arrDt.setMinutes(trip[8]);
        res.reservationFor.arrivalTime = arrDt;
        res.reservationFor.departureStation.name = trip[1];
        res.reservationFor.departureStation.identifier = 'uk:' + trip[2];
        res.reservationFor.arrivalStation.name = trip[3];
        res.reservationFor.arrivalStation.identifier = 'uk:' + trip[4];
        reservations.push(res);
    }

    reservations[0].reservationFor.departureTime = JsonLd.readQDateTime(event, 'dtStart');
    reservations[reservations.length - 1].reservationFor.arrivalTime = JsonLd.readQDateTime(event, 'dtEnd');
    return reservations;
}
