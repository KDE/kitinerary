/*
   SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
   SPDX-License-Identifier: LGPL-2.0-or-later
*/

function parseTicket(content, node, triggerNode) {
    const text = content.text;
    const ref = text.match(/(?:BOOKING REFERENCE|RÉSERVATION) +([A-Z0-9]{6})\n/)[1];

    // not really a valid IATA BCBP, but we can use parts of it at least
    const bcbp = triggerNode.content.substr(0, 22).match(/^M(\d)([^\/]+)\/(.*) */);
    var person = JsonLd.newObject("Person");
    person.familyName = bcbp[2];
    person.givenName = bcbp[3];

    var reservations = [];
    var idx = 0;
    while (true) {
        const leg = text.substr(idx).match(/(\d\d:\d\d) *(\d\d:\d\d)(?:.*)?\n *(\d\d[A-Z]{3}) +(.*?)  +(.*?)  +([A-Z0-9]{2}\d{4}).*\n *([A-Z]{3})  +([A-Z]{3}).*\n.*?(?:  +Siège\/Seat : ([0-9A-Z]+))?\n(?:.*\n)?.*(?:Flight operated by|Vol effectué par) (.*)\n/);
        if (!leg) {
            break;
        }
        idx += leg.index + leg[0].length;

        var res;
        if (leg[10] == 'SNCF') {
            res = JsonLd.newTrainReservation();
            res.reservationFor.departureStation.name = leg[4];
            res.reservationFor.departureStation.identifier = 'iata:' + leg[7];
            res.reservationFor.arrivalStation.name = leg[5];
            res.reservationFor.arrivalStation.identifier = 'iata:' + leg[8];
            res.reservationFor.trainNumber = leg[6];
            res.reservationFor.provider.name = leg[10];
        } else {
            res = JsonLd.newFlightReservation();
            res.reservationFor.departureAirport.name = leg[4];
            res.reservationFor.departureAirport.iataCode = leg[7];
            res.reservationFor.arrivalAirport.name = leg[5];
            res.reservationFor.arrivalAirport.iataCode = leg[8];
            res.reservationFor.flightNumber = leg[6];
            res.reservationFor.airline.name = leg[10];
            res.airplaneSeat = leg[9];
        }

        res.reservationFor.departureTime = JsonLd.toDateTime(leg[1] + ' ' + leg[3], "hh:mm ddMMM", "en");
        res.reservationFor.arrivalTime = JsonLd.toDateTime(leg[2] + ' ' + leg[3], "hh:mm ddMMM", "en");
        res.reservationNumber = ref;
        res.underName = person;
        res.reservedTicket.ticketToken = "aztec:" + triggerNode.content;
        reservations.push(res);
    }

    return reservations;
}
