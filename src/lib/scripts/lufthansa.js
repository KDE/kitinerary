/*
   SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
   SPDX-License-Identifier: LGPL-2.0-or-later
*/

function extractEvent(ev) {
    let res = JsonLd.newFlightReservation();

    // force UTC, otherwise we lose the timezone due to JS converting to the local TZ
    res.reservationFor.departureTime = ev.dtStart.toJSON();
    res.reservationFor.arrivalTime = ev.dtEnd.toJSON();

    const flight = ev.description.match(/Flight no: (\S{2}) (\d+)/);
    res.reservationFor.airline.iataCode = flight[1];
    res.reservationFor.flightNumber = flight[2];
    res.reservationFor.airline.name = ev.description.match(/operated by: (.*)/)[1];

    const dep = ev.location.match(/from (.*\(([A-Z]{3})\).*)/);
    res.reservationFor.departureAirport.name = dep[1];
    res.reservationFor.departureAirport.iataCode = dep[2];

    const arr = ev.summary.match(/to (.*\(([A-Z]{3})\).*)/);
    res.reservationFor.arrivalAirport.name = arr[1];
    res.reservationFor.arrivalAirport.iataCode = arr[2];

    res.reservationNumber = ev.description.match(/Reservation code: (.*)/)[1];
    return res;
}

function extractBoardingPass(iata, node, pdfNode) {
    let res = node.result[0];
    const text = pdfNode.content.pages[node.location].text;
    const boarding = text.match(/(\d\d:\d\d) (?:GROUP (\S+))?  +\d+[A-Z]/);
    res.reservationFor.boardingTime = JsonLd.toDateTime(boarding[1], 'hh:mm', 'en');
    res.boardingGroup = boarding[2];
    return res;
}
