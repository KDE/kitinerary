/*
   SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
   SPDX-License-Identifier: LGPL-2.0-or-later
*/

function fixAddress(content, node) {
    if (node.result.length != 1) {
        return;
    }
    // streetAddress duplicates city and zip code without proper separation in
    // about half their emails...
    var res = node.result[0];
    var addr = res.reservationFor.location.address;
    if (addr.streetAddress.endsWith(addr.addressLocality)) {
        addr.streetAddress = addr.streetAddress.substr(0, addr.streetAddress.length - addr.addressLocality.length).trim();
    }
    if (addr.streetAddress.endsWith(addr.postalCode)) {
        addr.streetAddress = addr.streetAddress.substr(0, addr.streetAddress.length - addr.postalCode.length);
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