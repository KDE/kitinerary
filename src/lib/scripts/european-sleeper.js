/*
   SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
   SPDX-License-Identifier: LGPL-2.0-or-later
*/

function parseTicket(pdf, node, triggerNode) {
    const text = pdf.pages[triggerNode.location].text;
    let res = triggerNode.result[0];
    res.reservationFor.trainNumber = text.match(/er ([A-Z]+ \d+)\n/)[1];
    res.reservedTicket.ticketNumber = res.reservationNumber;
    res.reservationNumber = text.match(/code ([A-Z0-9]{4}-[A-Z0-9]{4})\n/)[1];
    const leg = text.match(/(\S.*\S)  +(\d\d:\d\d)  +(\S.*\S)  +(\d\d:\d\d)\n/);
    res.reservationFor.departureDay = undefined; // off by one in their ERA FCB data...
    res.reservationFor.departureTime = JsonLd.toDateTime(leg[1] + ' ' + leg[2], 'd MMM yyyy hh:mm', ['en', 'nl']);
    res.reservationFor.arrivalTime = JsonLd.toDateTime(leg[3] + ' ' + leg[4], 'd MMM yyyy hh:mm', ['en', 'nl']);
    const seat = text.match(/(?:Carriage|Rijtuig) (.*) (?:Place|Plaats) (.*)\n/);
    if (seat) {
        res.reservedTicket.ticketedSeat.seatSection = seat[1];
        res.reservedTicket.ticketedSeat.seatNumber = seat[2];
    }
    return res;
}
