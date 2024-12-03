/*
   SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
   SPDX-License-Identifier: LGPL-2.0-or-later
*/

function parseTicket(pdf, node, triggerNode) {
    const text = pdf.pages[triggerNode.location].text;
    let res = triggerNode.result[0];
    res.reservationFor.trainNumber = text.match(/(?:Train number|Treinnummer|Zugnummer|Numéro du train) ([A-Z]+ \d+)\n/)[1];
    res.reservedTicket.ticketNumber = res.reservationNumber;
    res.reservationNumber = text.match(/(?:code|nummer|Code de réservation|Code du bilet) ([A-Z0-9]{4}-[A-Z0-9]{4})\n/)[1];
    const leg = text.match(/(\S.*\S)  +(\d\d:\d\d)  +(\S.*\S)  +(\d\d:\d\d)\n/);
    res.reservationFor.departureDay = undefined; // off by one in their ERA FCB data...
    res.reservationFor.departureTime = JsonLd.toDateTime(leg[1].replace('.', '') + ' ' + leg[2], 'd MMM yyyy hh:mm', ['en', 'nl', 'de', 'fr']);
    res.reservationFor.arrivalTime = JsonLd.toDateTime(leg[3].replace('.', '') + ' ' + leg[4], 'd MMM yyyy hh:mm', ['en', 'nl', 'de', 'fr']);
    const seat = text.match(/(?:Carriage|Rijtuig|Wagen|Voiture) (.*) (?:Place|Plaats|Platz) (.*)\n/);
    if (seat) {
        res.reservedTicket.ticketedSeat.seatSection = seat[1];
        res.reservedTicket.ticketedSeat.seatNumber = seat[2];
    }
    return res;
}
