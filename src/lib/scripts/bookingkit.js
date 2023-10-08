/*
   SPDX-FileCopyrightText: 2023 Kai Uwe Broulik <kde@broulik.de>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

function main(pass, node)
{
    let res = node.result[0];

    const eventName = pass.field["event"];
    res.reservationFor.name = eventName.value

    res.reservationFor.description = pass.description;

    res.reservationFor.location = JsonLd.newObject("Place");
    // This isn't technically correct but close enough most of the time.
    res.reservationFor.location.name = pass.organizationName;

    const eventTicketType = pass.field["event-ticket-type"];
    res.reservedTicket.name = eventTicketType.value;

    // Bookingkit returns UTC time but the displayed time is actually local.
    if (res.reservationFor.startDate.endsWith("Z")) {
        res.reservationFor.startDate = res.reservationFor.startDate.slice(0, -1); // chop(1)
    }

    return res;
}

function parsePdf(pdf, node, barcode) {
    const page = pdf.pages[barcode.location];
    const text = page.textInRect(0.0, 0.0, 0.9, 0.3);
    console.log(text);
    let res = JsonLd.newEventReservation();
    res.reservationFor.name = text.match(/^ *(\S.*)\n/)[1];
    res.reservedTicket.ticketToken = 'qrCode:' + barcode.content;
    res.reservationNumber = barcode.content;

    const dts = text.match(/(\d\d\.\d\d\.\d{4}).*\n *(\d\d\.\d\d\.\d{4})\n.*\n *(\d\d:\d\d).*(\d\d:\d\d)/);
    if (dts) {
        res.reservationFor.startDate = JsonLd.toDateTime(dts[1] + dts[3], 'dd.MM.yyyyhh:mm', 'en');
        res.reservationFor.endDate = JsonLd.toDateTime(dts[2] + dts[4], 'dd.MM.yyyyhh:mm', 'en');
    } else {
        const dt = text.match(/(\d\d\.\d\d\.\d{4}).*\n *(\d\d:\d\d)/);
        res.reservationFor.startDate = JsonLd.toDateTime(dt[1] + dt[2], 'dd.MM.yyyyhh:mm', 'en');
    }

    const loc = text.match(/(\S.*), (.*), (.*)\n *(.*)\n *(.*)\n.*##/);
    res.reservationFor.location.address.streetAddress = loc[1];
    res.reservationFor.location.address.addressLocality = loc[2];
    res.reservationFor.location.address.addressCountry = loc[3];
    res.reservedTicket.name = loc[4];
    res.underName.name = loc[5];
    return res;
}
