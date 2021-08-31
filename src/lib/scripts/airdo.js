/*
   SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
   SPDX-License-Identifier: LGPL-2.0-or-later
*/

function parseConfirmation(text, node) {
    var flightRes = new Array();
    var idx = 0;
    while (true) {
        const flight = text.substr(idx).match(/\[\d+\]\n(\d{1,2} \w+)\(.*\) ([A-Z0-9]+) (\d+)\n(.*) - (.*)\n.*?(\d{1,2}:\d{2})-(\d{1,2}:\d{2})/);
        if (!flight) {
            break;
        }
        idx += flight.index + flight[0].length;

        var f = JsonLd.newFlightReservation();
        f.reservationFor.departureAirport.name = flight[4];
        f.reservationFor.arrivalAirport.name = flight[5];
        f.reservationFor.departureTime = JsonLd.toDateTime(flight[1] + flight[6], "d MMMMhh:mm", "en");
        f.reservationFor.arrivalTime = JsonLd.toDateTime(flight[1] + flight[7], "d MMMMhh:mm", "en");
        f.reservationFor.airline.iataCode = flight[2];
        f.reservationFor.flightNumber = flight[3];
        flightRes.push(f);
    }

    const seats = text.match(/\[Seat Number\]([\s\S]+?)\n\[/);
    if (!seats) {
        return flightRes;
    }
    var reservations = new Array();
    for (seat of seats[1].split(/\n/)) {
        const passenger = seat.match(/(.*) (\d+[A-Z])[\n\[]/);
        if (!passenger) {
            continue;
        }
        for (flight of flightRes) {
            var res = JsonLd.clone(flight);
            res.underName.name = passenger[1];
            res.airplaneSeat = passenger[2];
            reservations.push(res);
        }
    }
    return reservations;
}
