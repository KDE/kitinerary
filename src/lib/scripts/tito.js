// SPDX-FileCopyrightText: 2024 Volker Krause <vkrause@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

function parseDate(res, date) {
    if (dt = date.match(/(\S+ \d{1,2})\S{2}, (\d{4})/)) {
        res.reservationFor.startDate = JsonLd.toDateTime(dt[1] + dt[2], "MMMM ddyyyy", "en");
    } else if (dt = date.match(/(\S+) (\d{1,2})\S{2}\S(\d{1,2})\S{2}, (\d{4})/)) {
        console.log(dt);
        res.reservationFor.startDate = JsonLd.toDateTime(dt[1] + ' ' + dt[2] + ' ' + dt[4], "MMMM d yyyy", "en");
        res.reservationFor.endDate = JsonLd.toDateTime(dt[1] + ' ' + dt[3] + ' ' + dt[4], "MMMM d yyyy", "en");
    }
}

function extractPass(pass, node) {
    let res = node.result[0];
    res.reservationFor.name = pass.logoText;
    res.reservedTicket.name = pass.field['ticket'].value;
    res.reservationFor.location = {
        '@type': 'Place',
        name: pass.field['location'].value
    };
    if (pass.field['reference'])
        res.reservationNumber = pass.field['reference'].value;
    res.underName = {
        '@type': 'Person',
        name: pass.secondaryFields.find(item => item.key === 'name').value,
        email: pass.field['email'] ? pass.field['email'].value : null
    }
    res.provider = {
        '@type': 'Organization',
        email: pass.field['contact_email'] ? pass.field['contact_email'].value : null
    };

    parseDate(res, pass.field['date'].value);
    return res;
}

function extractPdf(pdf, node, barcode) {
    const page = pdf.pages[barcode.location];
    const topRight = page.textInRect(0.5, 0.0, 1.0, 0.5);
    const ev = topRight.match(/([\S\s]+)\n(.*, \d{4})\n([\S\s]+)/);

    let res = JsonLd.newEventReservation();
    res.reservationFor.name = ev[1];
    parseDate(res, ev[2]);
    res.reservationFor.location.name = ev[3];
    res.reservedTicket.ticketToken = 'qrCode:' + barcode.content;

    const left = page.textInRect(0.0, 0.0, 0.5, 1.0);
    res.underName.name = left.match(/TICKET HOLDER\n(.*)\n/)[1];
    res.reservationNumber = left.match(/REFERENCE\n(.*)\n/)[1];
    res.reservedTicket.name = left.match(/TICKET\n(.*)\n/)[1];
    const org = left.match(/EVENT HOMEPAGE\n *([\S\s]+)\n.*EMAIL\n *(\S.*)\n/);
    res.reservationFor.url = org[1].replace(/\n */, '');
    res.provider = { '@type': 'Organization', email: org[2] };
    return res;
}
