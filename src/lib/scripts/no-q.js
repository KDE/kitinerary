/*
   SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
   SPDX-License-Identifier: LGPL-2.0-or-later
*/

function extractMail(content, node) {
    let res = node.result[0];
    res.reservedTicket = {
        '@type': 'Ticket',
        ticketToken: 'qrcode:' + node.findChildNodes({ scope: "Descendants", mimeType: "text/plain", match: "^[0-9a-z-]{16,}$" })[0].content 
    };
    res.cancelReservationUrl = res.reservationFor.url;
    res.reservationFor.url = undefined;
    const addr = res.reservationFor.location.name.split(/, /);
    res.reservationFor.location.address = {
        streetAddress: addr[0],
        addressCountry: addr[addr.length - 1],
        postalCode: addr[addr.length - 2],
        addressLocality: addr[addr.length - 3]
    };
    res.reservationFor.location.name = res.reservationFor.name;
    res.reservationFor.name = res.reservationFor.description.match(/^.*?: (.*)\n/)[1];
    return res;
}
