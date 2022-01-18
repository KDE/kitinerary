/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

function parseBooking(content) {
    var baseRes = JsonLd.newFlightReservation();
    const flight = content.match(/(\d{1,2}\w{3}\d{4}) Flight No.([A-Z0-9 ]+) \[.*:([^\]]+)\]\n.*from (.*) at (\d{2}:\d{2}) -> .* at (.*) at (\d{2}:\d{2})/);
    baseRes.reservationNumber = flight[3];
    baseRes.reservationFor.flightNumber = flight[2];
    baseRes.reservationFor.departureAirport.name = flight[4];
    baseRes.reservationFor.departureTime = JsonLd.toDateTime(flight[1] + flight[5], "ddMMMyyyyhh:mm", "en");
    baseRes.reservationFor.arrivalAirport.name = flight[6];
    baseRes.reservationFor.arrivalTime = JsonLd.toDateTime(flight[1] + flight[7], "ddMMMyyyyhh:mm", "en");

    var reservations = new Array();
    const passengers = content.match(/-Passengers\n([\s\S]*?)\n-/)[1].split(/\n/);
    for (const passenger of passengers) {
        const name = passenger.match(/M[a-z]+\.(.*)\(/);
        if (!name) {
            break;
        }
        var res = JsonLd.clone(baseRes);
        res.underName.name = name[1];
        reservations.push(res);
    }

    return reservations;
}
