/*
   SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
   SPDX-License-Identifier: LGPL-2.0-or-later
*/

function parsePdf(pdf, node, triggerNode) {
    const text = pdf.pages[triggerNode.location].text;
    const legs = text.split(/.* -> .*\n/).slice(1);
    let reservations = [];
    for (leg of legs) {
        let res = JsonLd.newBusReservation();
        res.reservationFor.departureBusStop.name = leg.match(/Origin station +(.*)/)[1];
        res.reservationFor.departureTime = JsonLd.toDateTime(leg.match(/Departure date and time +(.*)/)[1], 'dd.M.yyyy hh:mm', 'cz');
        res.reservationFor.departurePlatform = leg.match(/Platform +(.*)/)[1];
        res.reservationFor.arrivalBusStop.name = leg.match(/Destination station +(.*)/)[1];
        res.reservationFor.arrivalTime = JsonLd.toDateTime(leg.match(/Arrival date and time +(.*)/)[1], 'dd.M.yyyy hh:mm', 'cz');
        res.reservationNumber = triggerNode.content.match(/s:4:"code";s:\d+:"(.*?)"/)[1];
        res.underName.name = triggerNode.content.match(/s:8:"fullName";s:\d+:"(.*?)"/)[1];
        res.reservedTicket.ticketToken = 'qrcode:' + triggerNode.content;
        reservations.push(res);
        res.reservedTicket.ticketedSeat.seatNumber = leg.match(/Seat +(.*?) \(/)[1];
    }
    // while (true) {
    return reservations;
}
