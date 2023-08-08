/*
   SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
   SPDX-License-Identifier: LGPL-2.0-or-later
*/

function extractEvent(event, node) {
    let res = JsonLd.newEventReservation();
    res.reservationFor = node.result[0];
    res.reservationFor.description = undefined;
    res.reservationFor.url = undefined;
    const desc = event.description.match(/Buchungscode: *(\d+)\n(?:.*\n)*?(.*)\n(.*)\n\n/);
    res.reservedTicket.ticketToken = 'qrCode:' + desc[1];
    res.reservationFor.location.address = {
        streetAddress: desc[2],
        addressLocality: desc[3]
    };
    return res;
}
