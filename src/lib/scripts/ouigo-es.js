/*
   SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
   SPDX-License-Identifier: LGPL-2.0-or-later
*/

// barcode format
// 13x ticket number
// 5x ?
// 5x train number
// 15x departure time (dd/MM/yyyyhh:mm)
// 7x departure UIC station code
// 7x arrival UIC station code
// 3x coach number
// 3x seat number
// more stuff - signature?
function parseBarcode(content) {
    let res = JsonLd.newTrainReservation();
    res.reservedTicket.ticketToken = 'aztec:' + content;
    res.reservedTicket.ticketNumber = content.substr(0, 13);
    res.reservationFor.trainNumber = content.substr(18, 5);
    res.reservationFor.departureTime = JsonLd.toDateTime(content.substr(23, 15), 'dd/MM/yyyyhh:mm', 'es');
    res.reservationFor.departureStation.identifier = 'uic:71' + content.substr(40, 5);
    res.reservationFor.departureStation.name =  content.substr(38, 7);
    res.reservationFor.arrivalStation.identifier = 'uic:71' + content.substr(47, 5);
    res.reservationFor.arrivalStation.name = content.substr(45, 7);
    res.reservedTicket.ticketedSeat.seatSection = content.substr(52, 3);
    res.reservedTicket.ticketedSeat.seatNumber = content.substr(55, 3);
    return res;
}
