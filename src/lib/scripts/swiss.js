/*
   SPDX-FileCopyrightText: 2017-2022 Volker Krause <vkrause@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

function main(text) {
    var reservations = new Array();
    var bookingRef = text.match(/Buchungsreferenz: ([A-Z0-9]{6})/);

    var pos = 0;
    while (true) {
        var flight = text.substr(pos).match(/Hinflug|Rückflug/);
        if (!flight)
            break;
        var index = flight.index + flight[0].length;

        var res = JsonLd.newFlightReservation();
        res.reservationNumber = bookingRef[1];

        var leg = text.substr(pos + index).match(/  +(.+?) \(([A-Z]{3})\) - (.+?) \(([A-Z]{3})\) +[A-Z][a-z] (\d{2}.\d{2}.\d{4}) +(\d{2}:\d{2}) +[A-Z]{3} +(\d{2}:\d{2})  .*?  ([A-Z0-9]{2}) (\d{3,4})/);
        if (!leg)
            break;
        index += leg.index + leg[0].length;
        res.reservationFor.departureAirport.name = leg[1];
        res.reservationFor.departureAirport.iataCode = leg[2];
        res.reservationFor.arrivalAirport.name = leg[3];
        res.reservationFor.arrivalAirport.iataCode = leg[4];
        res.reservationFor.departureTime = JsonLd.toDateTime(leg[5] + ' ' + leg[6], "dd.MM.yyyy hh:mm", "en");
        res.reservationFor.arrivalTime = JsonLd.toDateTime(leg[5] + ' ' + leg[7], "dd.MM.yyyy hh:mm", "en");
        res.reservationFor.flightNumber = leg[9];
        res.reservationFor.airline.iataCode = leg[8];
        // TODO: parse the operated by part to fill in airline name

        reservations.push(res);
        if (index == 0)
            break;
        pos += index;
    }

    return reservations;
}

function parseEvent(event)
{
    let res = JsonLd.newFlightReservation();
    res.reservationFor.departureTime = JsonLd.readQDateTime(event, 'dtStart');
    res.reservationFor.arrivalTime = JsonLd.readQDateTime(event, 'dtEnd');

    res.reservationNumber = event.description.match(/Booking reference: (.*)\n/)[1];
    const flight = event.description.match(/Airline: (.*)\nFlight number: (.{2}) (.*)\n/);
    res.reservationFor.airline.name = flight[1];
    res.reservationFor.airline.iataCode = flight[2];
    res.reservationFor.flightNumber = flight[3];

    res.reservationFor.departureAirport.name = event.location;
    res.reservationFor.arrivalAirport.name = event.description.match(/At: (.*)\n/)[1];

    return res;
}
