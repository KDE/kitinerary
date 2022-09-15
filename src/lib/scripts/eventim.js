/*
   SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
   SPDX-License-Identifier: LGPL-2.0-or-later
*/

function parsePkPass(pass, node) {
    let res = node.result[0];
    console.log(pass);
    res.reservationFor.name = pass.logoText;
    const loc = pass.field['KEY_LOCATION'].value.split('\n');
    res.reservationFor.location = JsonLd.newObject('Place');
    res.reservationFor.location.name = loc[0];
    res.reservedTicket.ticketedSeat = JsonLd.newObject('Seat');
    res.reservationFor.location.address = JsonLd.newObject('PostalAddress');
    const addr = loc[1].match(/(.*), (.*?)$/);
    res.reservationFor.location.address.streetAddress = addr[1];
    res.reservationFor.location.address.addressLocality = addr[2];
    res.reservedTicket.ticketedSeat.seatNumber = pass.field['KEY_BACK_SEATLINE'].value;
    return res;
}
