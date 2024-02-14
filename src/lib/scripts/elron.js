/*
   SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
   SPDX-License-Identifier: LGPL-2.0-or-later
*/

function parsePdfTicket(pdf, node, triggerNode) {
    const text = pdf.pages[triggerNode.location].text;
    let res = JsonLd.newTrainReservation();
    const trip = text.match(/(\d\d:\d\d) (\S.*) - (\d\d:\d\d) (\S.*)  +(\d\d\.\d\d\.\d{4})\n *(\S.*?) /);
    res.reservationFor.departureTime = JsonLd.toDateTime(trip[5] + ' ' + trip[1], 'dd.MM.yyyy hh:mm', 'ee');
    res.reservationFor.departureStation.name = trip[2];
    res.reservationFor.arrivalTime = JsonLd.toDateTime(trip[5] + ' ' + trip[3], 'dd.MM.yyyy hh:mm', 'ee');
    res.reservationFor.arrivalStation.name = trip[4];
    res.reservationFor.trainNumber = trip[6];
    res.reservedTicket.ticketToken = 'qrcode:' + triggerNode.content;
    const seat = text.match(/Place: (\S.*)\n/);
    if (seat)
        res.reservedTicket.ticketedSeat.seatNumber = seat[1];
    return res;
}
