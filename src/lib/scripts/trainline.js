/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

function parseEvent(event) {
    let res = JsonLd.newTrainReservation();
    res.reservationFor.departureTime = JsonLd.readQDateTime(event, 'dtStart');
    res.reservationFor.arrivalTime = JsonLd.readQDateTime(event, 'dtEnd');
    res.reservationFor.departureStation.name = event.location;
    res.reservationFor.arrivalStation.name = event.summary.match(/.* → (.*) \[/)[1];
    return res;
}
