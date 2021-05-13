/*
   SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

function parseTicket(ticket) {
    var res = JsonLd.newTrainReservation();
    res.reservationNumber = ticket.pnr;
    res.reservedTicket.ticketToken = "aztectBin:" + Barcode.toBase64(ticket.rawData);
    res.underName.name = ticket.ticketLayout.text(0, 0, 72, 1);
    res.reservationFor.departureTime = JsonLd.toDateTime(ticket.ticketLayout.text(3, 0, 72, 1).match(/([\d\.: ]+)/)[1], "dd.MM.yyyy hh:mm", "de");
    res.reservationFor.departureStation.name = ticket.ticketLayout.text(7, 0, 72, 1);
    res.reservationFor.arrivalStation.name = ticket.ticketLayout.text(8, 0, 72, 1);
    return res;
}
