/*
   SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
   SPDX-License-Identifier: LGPL-2.0-or-later
*/

function parseTicket(pdf, node, triggerNode) {
    const text = pdf.pages[triggerNode.location].text;
    let res = triggerNode.result[0];
    res.reservationFor.trainNumber = text.match(/Train number (.*)\n/)[1];
    res.reservedTicket.ticketNumber = res.reservationNumber;
    res.reservationNumber = text.match(/Booking code (.*)\n/)[1];
    const leg = text.match(/(\S.*\S)  +(\d\d:\d\d)  +(\S.*\S)  +(\d\d:\d\d)\n/);
    res.reservationFor.departureTime = JsonLd.toDateTime(leg[1] + ' ' + leg[2], 'd MMM yyyy hh:mm', 'en');
    res.reservationFor.arrivalTime = JsonLd.toDateTime(leg[3] + ' ' + leg[4], 'd MMM yyyy hh:mm', 'en');
    return res;
}