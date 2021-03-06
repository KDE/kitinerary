/*
   SPDX-FileCopyrightText: 2017 Volker Krause <vkrause@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

function main(text) {
    var reservations = new Array();
    var bookingRef = text.match(/BOOKING REF: ([A-Z0-9]{6})/);

    var pos = 0;
    while (true) {
        var flightHeader = text.substr(pos).match(/FLIGHT *([A-Z0-9]{2}) ([0-9]{3,4}) - ([A-Za-z0-9 ]*?)  .*([0-9]{4})\n/);
        if (!flightHeader)
            break;
        var idx = flightHeader.index + flightHeader[0].length;

        var res = JsonLd.newFlightReservation();
        res.reservationNumber = bookingRef[1];
        res.reservationFor.airline.iataCode = flightHeader[1];
        res.reservationFor.airline.name = flightHeader[3];
        res.reservationFor.flightNumber = flightHeader[2];

        // TODO support line continuation for DEPARTURE/ARRIVAL
        var depLine = text.substr(pos + idx).match(/DEPARTURE: +(.*?) [ -] *([0-9]{2} [A-Z]{3}) ([0-9]{2}:[0-9]{2})/);
        if (!depLine)
            break;
        idx = depLine.index + depLine[0].length;
        res.reservationFor.departureAirport.name = depLine[1];
        res.reservationFor.departureTime = JsonLd.toDateTime(depLine[2] + ' ' + flightHeader[4] + ' ' + depLine[3], "dd MMM yyyy hh:mm", "en");

        var arrLine = text.substr(pos + idx).match(/ARRIVAL: +(.*?) [ -] *([0-9]{2} [A-Z]{3}) ([0-9]{2}:[0-9]{2})/);
        if (!arrLine)
            break;
        idx = arrLine.index + arrLine[0].length;
        res.reservationFor.arrivalAirport.name = arrLine[1];
        res.reservationFor.arrivalTime = JsonLd.toDateTime(arrLine[2] + ' ' + flightHeader[4] + ' ' + arrLine[3], "dd MMM yyyy hh:mm", "en");

        reservations.push(res);
        if (idx == 0)
            break;
        pos += idx;
    }

    return reservations;
}

function parseEvent(event)
{
    var res = JsonLd.newFlightReservation();

    // force UTC, otherwise we lose the timezone due to JS converting to the local TZ
    res.reservationFor.departureTime = event.dtStart.toJSON();
    res.reservationFor.arrivalTime = event.dtEnd.toJSON();
    res.reservationNumber = event.uid.substr(0, 6);

    var flight = event.description.match(/Flight: (.*) - (\S{2}) (\S{1,4})\n/);
    res.reservationFor.airline.name = flight[1];
    res.reservationFor.airline.iataCode = flight[2];
    res.reservationFor.flightNumber = flight[3];

    var from = event.description.match(/From: (.*)\n/);
    res.reservationFor.departureAirport.name = from[1];

    var to = event.description.match(/To: (.*)\n/);
    res.reservationFor.arrivalAirport.name = to[1];

    return res;
}
