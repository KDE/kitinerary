/*
   SPDX-FileCopyrightText: 2017 Volker Krause <vkrause@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

function main(text) {
    var reservations = new Array();
    var bookingRef = text.match(/(?:Individual reservation code|Individueller Buchungscode):\s*\*\* ([A-Z0-9]{6})/);

    var pos = 0;
    while (true) {
        var flightLine = text.substr(pos).match(/(?:Flight|Flug): ([0-9]{2}\.[0-9]{2}\.[0-9]{4})\s*\|\s*([A-Z0-9]{2}) ([0-9]{3,4}).*\n/);
        if (!flightLine)
            break;
        var idx = flightLine.index + flightLine[0].length;

        var res = JsonLd.newFlightReservation();
        res.reservationNumber = bookingRef[1];
        res.reservationFor.flightNumber = flightLine[3];

        res.reservationFor.airline.iataCode = flightLine[2];

        var opByLine = text.substr(pos + idx).match(/^\s*\* (?:operated by|durchgeführt von) (.*)\n/);
        if (opByLine) {
            idx += opByLine.index + opByLine[0].length;
            res.reservationFor.airline.name = opByLine[1];
        }

        var depLine = text.substr(pos + idx).match(/(?:Departure|Abflug):\s*([0-9]{2}:[0-9]{2})\s+(.*)\n/);
        if (!depLine)
            break;
        idx += depLine.index + depLine[0].length;
        res.reservationFor.departureTime = JsonLd.toDateTime(flightLine[1] + ' ' + depLine[1], "dd.MM.yyyy hh:mm", "en");
        res.reservationFor.departureAirport.name = depLine[2];

        var arrLine = text.substr(pos + idx).match(/(?:Arrival|Ankunft):\s*([0-9]{2}:[0-9]{2})\s+(.*)\n/);
        if (!arrLine)
            break;
        idx += arrLine.index + arrLine[0].length;
        res.reservationFor.arrivalTime = JsonLd.toDateTime(flightLine[1] + ' ' + arrLine[1], "dd.MM.yyyy hh:mm", "en");
        res.reservationFor.arrivalAirport.name = arrLine[2];

        reservations.push(res);
        if (idx == 0)
            break;
        pos += idx;
    }

    return reservations;
}
