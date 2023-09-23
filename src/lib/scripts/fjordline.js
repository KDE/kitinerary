/*
   SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
   SPDX-License-Identifier: LGPL-2.0-or-later
*/

function parseTicket(pdf, node, triggerNode) {
    const text = pdf.pages[triggerNode.location].text;
    if (!text.match(/Fjord Line/))
        return;
    let res = JsonLd.newBoatReservation();
    const trip = text.match(/(\d{2}.\d{2}.\d{4}) +(.*?) +(\d{2}\.\d{2}) +(.*) +(\d{2}\.\d{2}) +(.*?)  +(.*)/);
    res.reservationFor.departureBoatTerminal.name = trip[2];
    res.reservationFor.departureTime = JsonLd.toDateTime(trip[1] + ' ' + trip[3], 'dd.MM.yyyy hh.mm', 'no');
    res.reservationFor.arrivalBoatTerminal.name = trip[4];
    res.reservationFor.arrivalTime = JsonLd.toDateTime(trip[1] + ' ' + trip[5], 'dd.MM.yyyy hh.mm', 'no');
    res.reservationFor.boatName = trip[6];
    res.underName.name = trip[7];
    res.reservationNumber = triggerNode.content;
    res.reservedTicket.ticketToken = 'qrCode:' + triggerNode.content;
    return res;
}
