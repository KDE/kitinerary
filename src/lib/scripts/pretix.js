/*
   SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
   SPDX-License-Identifier: LGPL-2.0-or-later
*/

function parsePass(content, node) {
    var res = node.result[0];
    res.reservationFor.name = content.field['eventName'].value;
    res.reservationFor.startDate = JsonLd.toDateTime(content.field['doorsOpen'].value, ['dd.MM.yyyy hh:mm', 'dd.MM.yyyy'], 'de');
    res.reservationFor.endDate = JsonLd.toDateTime(content.field['doorsClose'].value, ['dd.MM.yyyy hh:mm', 'dd.MM.yyyy'], 'de');
    res.reservationFor.url = content.field['website'].value;
    res.underName = JsonLd.newObject('Person');
    res.underName.name = content.field['name'].value;
    res.reservationNumber = content.field['orderCode'].value;
    res.reservedTicket.name = content.field['ticket'].value;
    return res;
}

// only works for unstyled PDFs common for smaller events
function parsePdf(pdf, node, barcode) {
    let res = JsonLd.newEventReservation();
    const text = pdf.pages[barcode.location].textInRect(0.0, 0.0, 1.0, 0.4);
    const data = text.trim().split(/\n/);
    res.reservationFor.location.name = data[data.length - 2];
    res.reservationNumber = data[data.length - 1].match(/(\S+) /)[1];
    res.reservationFor.startDate = JsonLd.toDateTime(data[data.length - 3], ['dd.MM.yyyy hh:mm', 'yyyy-MM-dd hh:mm'], 'en');
    res.underName.name = data[data.length - 4];
    res.reservationFor.name = data.slice(0, data.length - 5).join(' ');
    res.reservedTicket.ticketToken = 'qrcode:' + barcode.content;
    return res;
}
