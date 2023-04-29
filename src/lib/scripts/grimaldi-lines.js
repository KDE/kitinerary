/*
   SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
   SPDX-License-Identifier: LGPL-2.0-or-later
*/

function parsePdfTicket(pdf, node, triggerNode) {
    const text = pdf.pages[triggerNode.location].text;
    let res = JsonLd.newBoatReservation();
    const dep = text.match(/Departure: (.*)  +(\d{2}\/\d{2}\/\d{2} \d{2}:\d{2})/);
    res.reservationFor.departureBoatTerminal.name = dep[1];
    res.reservationFor.departureTime = JsonLd.toDateTime(dep[2], 'dd/MM/yy hh:mm', 'it');
    const arr = text.match(/Arrival: (.*)  +(\d{2}\/\d{2}\/\d{2} \d{2}:\d{2})/);
    res.reservationFor.arrivalBoatTerminal.name = arr[1];
    res.reservationFor.arrivalTime = JsonLd.toDateTime(arr[2], 'dd/MM/yy hh:mm', 'it');
    res.reservationNumber = triggerNode.content;
    res.reservedTicket.ticketToken = 'barcode128:' + triggerNode.content;
    return res;
}
