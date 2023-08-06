/*
   SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
   SPDX-License-Identifier: LGPL-2.0-or-later
*/

function extractPdf(pdf, node) {
    let reservations = [];
    for (let i = 0; i < pdf.pageCount; ++i) {
        let res = JsonLd.newEventReservation();
        const barcodes = node.findChildNodes({ scope: "Descendants", mimeType: "text/plain", match: "^\\S+$" });
        for (const barcode of barcodes) {
            if (barcode.location == i) {
                res.reservedTicket.ticketToken = 'qrcode:' + barcode.content;
                break;
            }
        }
        const page = pdf.pages[i];
        const leftCol = page.textInRect(0.0, 0.0, 0.33, 1.0);
        const event = leftCol.match(/.*\n((?:.*\n)*.*)\n.*\n(.{3} \d\d, \d{4} \d{1,2}:\d\d ..)\n.*\n(.{3} \d\d, \d{4} \d{1,2}:\d\d ..)\n.*\n(.*)\n(.*)\n(.*)\n/);
        res.reservationFor.name = event[1].split('|')[0];
        res.reservationFor.startDate = JsonLd.toDateTime(event[2], 'MMM dd, yyyy h:mm A', 'en');
        res.reservationFor.endDate = JsonLd.toDateTime(event[3], 'MMM dd, yyyy h:mm A', 'en');
        res.reservationFor.location.name = event[4];
        res.reservationFor.location.address.streetAddress = event[5];
        res.reservationFor.location.address.addressLocality = event[6];
        reservations.push(res);
    }
    return reservations;
}
