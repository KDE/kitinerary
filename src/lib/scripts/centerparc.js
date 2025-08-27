// SPDX-FileCopyrightText: 2025 Volker Krause <vkrause@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

function extractPdf(pdf, node, barcode) {
    const text = pdf.pages[barcode.location].text;
    let res = JsonLd.newLodgingReservation();
    const name = text.match(/(.*)\n.*: (.*)\n/);
    res.reservationFor.name = name[2];
    res.underName.name = name[1];
    const dates = text.match(/(\d{2}\.\d{2}\.\d{4})\s+(\d{2}\.\d{2}\.\d{4})/);
    res.checkinTime = JsonLd.toDateTime(dates[1], "dd.MM.yyyy", "de");
    res.checkoutTime = JsonLd.toDateTime(dates[2], "dd.MM.yyyy", "de");
    res.reservedTicket = {
        '@type': 'Ticket',
        ticketToken: 'qrcode:' + barcode.content
    };
    return res;
}
