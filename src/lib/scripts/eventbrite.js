/*
   SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
   SPDX-License-Identifier: LGPL-2.0-or-later
*/

function fixAddress(content, node) {
    if (node.result.length != 1) {
        return;
    }
    var res = node.result[0];
    // location can be a string rather than a place object
    if (typeof res.reservationFor.location === "string") {
        var place = JsonLd.newObject("Place");
        place.name = res.reservationFor.location;
        res.reservationFor.location = place;
    }

    // streetAddress duplicates city and zip code without proper separation in
    // about half their emails...
    if (!res.reservationFor.location.address) {
        return res;
    }
    var addr = res.reservationFor.location.address;
    for (var i of [0, 1]) {
        if (addr.streetAddress.endsWith(addr.addressLocality)) {
            addr.streetAddress = addr.streetAddress.substr(0, addr.streetAddress.length - addr.addressLocality.length).trim();
        }
        if (addr.streetAddress.endsWith(addr.postalCode)) {
            addr.streetAddress = addr.streetAddress.substr(0, addr.streetAddress.length - addr.postalCode.length).trim();
        }
    }
    res.reservationFor.location.address = addr;
    return res;
}

function findBarcode(content, node) {
    if (node.result.length != 1) {
        return;
    }
    const pdfs = node.findChildNodes({ mimeType: "application/pdf", scope: "Descendants" });
    if (pdfs.length == 1) {
        const images = pdfs[0].findChildNodes({ mimeType: "internal/qimage", scope: "Descendants" });
        var barcode = undefined;
        for (const image of images) {
            if (!image.childNodes || image.childNodes.length != 1 || image.childNodes[0].mimeType != "text/plain") {
                continue;
            }
            if (barcode && barcode != image.childNodes[0].content) {
                return;
            }
            barcode = image.childNodes[0].content;
        }
        if (barcode) {
            var res = node.result[0];
            if (res.reservedTicket == undefined)
                res.reservedTicket = JsonLd.newObject("Ticket");
            res.reservedTicket.ticketToken = "qrCode:" + barcode;
            return res;
        }
    }
}

function parsePdf(pdf, node, triggerNode) {
    const page = pdf.pages[triggerNode.location];
    let res = JsonLd.newEventReservation();
    res.reservationNumber = triggerNode.content.substr(0, 10);
    res.reservedTicket.ticketToken = 'qrCode:' + triggerNode.content;
    res.reservationFor.name = page.textInRect(0.0, 0.12, 1.0, 0.18);
    const multiDay = page.textInRect(0.0, 0.27, 1.0, 0.3).match(/(\d\d\.? \S+ \d{4}).*? (\d\d:\d\d).*?(\d\d\.? \S+ \d{4}).*(\d\d:\d\d)/);
    const oneDay = page.textInRect(0.0, 0.27, 1.0, 0.3).match(/(\d\d\.? \S+ \d{4}).*? (\d\d:\d\d).*?(\d\d:\d\d)/);
    res.reservationFor.startDate = JsonLd.toDateTime(oneDay[1] + ' ' + oneDay[2], ['dd MMMM yyyy hh:mm', 'dd. MMMM yyyy hh:mm'], ['en', 'de', 'fr']);
    res.reservationFor.endDate = JsonLd.toDateTime((multiDay ? multiDay[3] : oneDay[1]) + ' ' + oneDay[3], ['dd MMMM yyyy hh:mm', 'dd. MMMM yyyy hh:mm'], ['en', 'de', 'fr']);
    const addr = page.textInRect(0.0, 0.2, 1.0, 0.27).split(',');
    res.reservationFor.location.name = addr[0];
    res.reservationFor.location.address.addressCountry = addr[addr.length - 1];
    res.reservationFor.location.address.addressLocality = addr[addr.length - 2];
    res.reservationFor.location.address.streetAddress = addr[addr.length - 3];
    const name = page.text.match(/  Name\n.*  (.*)/);
    if (name) {
        res.underName.name = name[1];
    }
    return res;
}
