/*
   SPDX-FileCopyrightText: 2017 Volker Krause <vkrause@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

function main(text) {
    var reservations = new Array();
    var bookingRef = text.match(/(?:Individual reservation code|Individueller Buchungscode):?\s*(?:\(beim Check-In angeben\))?\s*\*\* ([A-Z0-9]{6})/);

    var pos = 0;
    while (true) {
        var flightLine = text.substr(pos).match(/(?:Flight|Flug): ([0-9]{2}\.[0-9]{2}\.[0-9]{4})\s*\|\s*(?:Flugnummer\:)?\s*([A-Z0-9]{2}) ([0-9]{3,4}).*\n/);
        if (!flightLine)
            break;
        var idx = flightLine.index + flightLine[0].length;

        var res = JsonLd.newFlightReservation();
        res.reservationNumber = bookingRef[1];
        res.reservationFor.flightNumber = flightLine[3];

        res.reservationFor.airline.iataCode = flightLine[2];

        var opByLine = text.substr(pos + idx).match(/^\s*\* (?:[Oo]perated by|durchgeführt von) (.*)\n/);
        if (opByLine) {
            idx += opByLine.index + opByLine[0].length;
            res.reservationFor.airline.name = opByLine[1];
        }

        var depLine = text.substr(pos + idx).match(/(?:Departure|Abflug):?\s*([0-9]{2}:[0-9]{2})\s+(.*)\n/);
        if (!depLine)
            break;
        idx += depLine.index + depLine[0].length;
        res.reservationFor.departureTime = JsonLd.toDateTime(flightLine[1] + ' ' + depLine[1], "dd.MM.yyyy hh:mm", "en");
        res.reservationFor.departureAirport.name = depLine[2];

        var arrLine = text.substr(pos + idx).match(/(?:Arrival|Ankunft):?\s*([0-9]{2}:[0-9]{2})\s+(.*)\n/);
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

function extractEvent(ev)
{
    let res = JsonLd.newFlightReservation();
    res.reservationFor.departureTime = JsonLd.readQDateTime(ev, 'dtStart');
    res.reservationFor.arrivalTime = JsonLd.readQDateTime(ev, 'dtEnd');
    const uid = ev.uid.match(/([A-Z0-9]{6})_([A-Z]{3})([A-Z]{3})/);
    res.reservationNumber = uid[1];
    res.reservationFor.departureAirport.iataCode = uid[2];
    res.reservationFor.arrivalAirport.iataCode = uid[3];
    const loc = ev.location.match(/(.*) \(([A-Z0-9]{2})(\d{1,4})\)/);
    res.reservationFor.departureAirport.name = loc[1];
    res.reservationFor.airline.iataCode = loc[2];
    res.reservationFor.flightNumber = loc[3];
    const arrName = ev.description.match(/(?:To|Nach): (.*)/);
    if (arrName)
        res.reservationFor.arrivalAirport.name = arrName[1];
    return res;
}
