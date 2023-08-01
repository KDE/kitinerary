/*
   SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
   SPDX-License-Identifier: LGPL-2.0-or-later
*/

function cleanText(s)
{
    return s.replace(/<\d+>/, " ");
}

// see https://community.kde.org/KDE_PIM/KItinerary/PKP_Barcode
function parseBarcode(content, node) {
    const layout = content.ticketLayout;
    let res = JsonLd.newTrainReservation();
    res.reservedTicket = node.result[0];
    res.reservedTicket.issuedBy = undefined; // invalid UIC company code
    res.reservedTicket.name = cleanText(layout.text(2, 0, 72, 1));
    res.reservationFor.departureTime = JsonLd.toDateTime(layout.text(10, 0, 72, 1) + ' ' + layout.text(11, 0, 72, 1), 'hh:mm dd.MM', 'pl');
    res.reservationFor.arrivalTime = JsonLd.toDateTime(layout.text(14, 0, 72, 1) + ' ' + layout.text(15, 0, 72, 1), 'hh:mm dd.MM', 'pl');
    res.reservedTicket.ticketedSeat = JsonLd.newObject('Seat');
    res.reservedTicket.ticketedSeat.seatingType = layout.text(16, 0, 72, 1);
    res.reservationFor.trainNumber = layout.text(18, 0, 72, 1);
    res.reservedTicket.ticketedSeat.seatNumber = cleanText(layout.text(23, 0, 72, 1));
    res.reservationNumber = layout.text(31, 0, 72, 1);
    res.reservationFor.departureStation.name = layout.text(41, 0, 72, 1);
    res.reservationFor.departureStation.identifier = 'pl:' + layout.text(12, 0, 72, 1);
    res.reservationFor.arrivalStation.name = layout.text(42, 0, 72, 1);
    res.reservationFor.arrivalStation.identifier = 'pl:' + layout.text(13, 0, 72, 1);
    res.totalPrice = layout.text(26, 0, 72, 1) / 100;
    res.priceCurrency = 'PLN';
    return res;
}
