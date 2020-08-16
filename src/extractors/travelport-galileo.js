/*
   SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

function parsePNR(text) {
    var reservations = new Array();

    var passengers = new Array();
    var pos = 0;
    while (true) {
        var passenger = text.substr(pos).match(/\d\.1(.+?)\/([\w\s]+?)(?:\*|  |\n)/);
        if (!passenger)
            break;
        pos += passenger.index + passenger[0].length;

        var person = JsonLd.newObject("Person");
        person.familyName = passenger[1];
        person.givenName = passenger[2];
        passengers.push(person);
    }

    pos = 0;
    var legs = new Array();
    while (true) {
        var leg = text.substr(pos).match(/\d. ([A-Z0-9]{2}) +(\d{1,4}) [A-Z]{1,2} {1,2}(\d{2}[A-Z]{3}) ([A-Z]{3})([A-Z]{3}) [A-Z]{2}\d  (\d{4})  [ #](\d{4}).*\n(?:\s*OPERATED BY (.*?)\n)?/);
        if (!leg)
            break;
        pos += leg.index + leg[0].length;

        var res = JsonLd.newFlightReservation();
        res.reservationFor.airline.iataCode = leg[1];
        res.reservationFor.airline.name = leg[8];
        res.reservationFor.flightNumber = leg[2];
        res.reservationFor.departureAirport.iataCode = leg[4];
        res.reservationFor.arrivalAirport.iataCode = leg[5];
        res.reservationFor.departureTime = JsonLd.toDateTime(leg[3] + leg[6], "ddMMMhhmm", "en");
        res.reservationFor.arrivalTime = JsonLd.toDateTime(leg[3] + leg[7], "ddMMMhhmm", "en");
        legs.push(res);
    }

    for (var i = 0; i < legs.length; ++i) {
        for (var j = 0; j < passengers.length; ++j) {
            var res = JsonLd.clone(legs[i]);
            res.underName = passengers[j];
            reservations.push(res);
        }
    }
    return reservations;
}
