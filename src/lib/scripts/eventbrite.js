/*
   SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
   SPDX-FileCopyrightText: 2025 David Pilarčík <meow@charliecat.space>
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
    const dtText = page.textInRect(0.0, 0.27, 1.0, 0.3);
    for (const rx of [/(\d\d\.? \S+ \d{4})/, /(\S+ \d{1,2}, \d{4})/]) {
        startDate = dtText.match(rx);
        if (startDate) {
            endDate = dtText.substr(startDate.index + startDate[0].length).match(rx);
            break;
        }
    }
    for (const rx of [/ (\d\d:\d\d)/, / (\d\d?:\d\d [AP]M)/]) {
        startTime = dtText.match(rx);
        if (startTime) {
            endTime = dtText.substr(startTime.index + startTime[0].length).match(rx);
            break;
        }
    }
    const timeFormats = ['dd MMMM yyyy hh:mm', 'dd. MMMM yyyy hh:mm', 'MMMM d, yyyy h:mm AP'];
    res.reservationFor.startDate = JsonLd.toDateTime(startDate[1] + ' ' + startTime[1], timeFormats , ['en', 'de', 'fr']);
    res.reservationFor.endDate = JsonLd.toDateTime((endDate ? endDate[1] : startDate[1]) + ' ' + endTime[1], timeFormats, ['en', 'de', 'fr']);
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

function parsePkPass(pass, node) {
    let res = node.result[0];
    console.log(JSON.stringify(pass, null, 4));
    console.log(JSON.stringify(res, null, 4))

    res.reservationFor.name = pass.field["event-name"].value
    res.reservationNumber = pass.field["order-id"]?.value.replace("#", "")
    res.provider = Object.assign(
        JsonLd.newObject('Organization'), 
        {
            name: pass.field["organizer-name"].value,
        }
    )
    res.reservationFor.startDate = pass.field["start-time"].value
    res.reservedTicket.underName = Object.assign(
        JsonLd.newObject('Person'),
        {
            name: pass.field["attendee-name"]?.value ?? pass.field["ticket-buyer-name"]?.value,
        }
    )
    res.reservedTicket.name = pass.field["ticket-type"].value

    // Example: "The Spot, Sky Park Bratislava - Bottova 2/A - 811 09 Bratislava - Slovakia"
    let location = pass.field["venue-name"].value.split(" - ")
    res.reservationFor.location = Object.assign(
        JsonLd.newObject('Place'),
        {
            name: location.length >= 4 ? location[0] : undefined,
            address: Object.assign(
                JsonLd.newObject('PostalAddress'),
                {
                    streetAddress: location.length >= 4 ? location[1] : location[0],
                    addressLocality: location.length >= 4 ? location[2] : location[1],
                    addressCountry: location.length >= 4 ? location[3] : location[2],
                }
            )
        }
    )

    return res
}