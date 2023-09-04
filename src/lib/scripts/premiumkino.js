/*
   SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
   SPDX-License-Identifier: LGPL-2.0-or-later
*/

function extractPass(pass) {
    let res = JsonLd.newEventReservation();
    res.reservationFor.startDate = pass.relevantDate;
    res.reservationFor.name = pass.field['TicketTitle'].value;
    res.reservationFor.location.name = pass.organizationName;
    const addr = pass.field['Back8'].value.split('\n');
    res.reservationFor.location.address.streetAddress = addr[1];
    res.reservationFor.location.address.addressLocality = addr[2];
    res.reservationFor.location.geo.latitude = pass.locations[0].latitude;
    res.reservationFor.location.geo.longitude = pass.locations[0].longitude;
    res.reservedTicket.ticketToken = 'qrCode:' + pass.barcodes[0].message;
    res.reservedTicket.ticketedSeat.seatSection = pass.field['Theatre'].value;
    res.reservedTicket.ticketedSeat.seatNumber = pass.field['Seats'].value;
    return res;
}
