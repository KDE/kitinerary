/*
   SPDX-FileCopyrightText: 2025 Volker Krause <vkrause@kde.org>
   SPDX-License-Identifier: LGPL-2.0-or-later
*/

function extractTicket(pdf, node, barcode) {
    let res = JsonLd.newEventReservation();
    res.reservedTicket.ticketToken = 'qrcode:' + barcode.content;

    const text = pdf.pages[barcode.location].text;
    const data = text.match(/(#O\d{10}-\d{5})\n+(.*)\n+(.*)\n+Event taking place ([\S\s]*)\.\n+From: .*, (\S+ \d{2}, \d{4}, \d{1,2}:\d\d:\d\d [AP]M).*\n+To: .* (\S+ \d{2}, \d{4}, \d{1,2}:\d\d:\d\d [AP]M)/);
    res.reservationNumber = data[1];
    res.reservationFor.name = data[2];
    res.reservedTicket.name = data[3];

    const addr = data[4].replace('\n', ' ').split(',');
    res.reservationFor.location.name = addr[0];
    res.reservationFor.location.address.streetAddress = addr[1];
    res.reservationFor.location.address.addressLocality = addr[addr.length - 3];
    res.reservationFor.location.address.postalCode = addr[addr.length - 2];
    res.reservationFor.location.address.addressCountry = addr[addr.length - 1];

    res.reservationFor.startDate = JsonLd.toDateTime(data[5], 'MMMM dd, yyyy, h:mm:ss AP', 'en');
    res.reservationFor.endDate = JsonLd.toDateTime(data[6], 'MMMM dd, yyyy, h:mm:ss AP', 'en');
    return res;
}
