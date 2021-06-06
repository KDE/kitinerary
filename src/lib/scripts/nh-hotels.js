/*
   SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

function lastChild(elem) {
    var child = elem.firstChild;
    while (!child.nextSibling.isNull) {
        child = child.nextSibling;
    }
    return child;
}

function parseDateTime(text) {
    var dt = text.match(/([\d/]+).*?([\d:]+)/);
    return JsonLd.toDateTime(dt[1] + dt[2], "dd/MM/yyyyhh:mm", "en");
}

function parseHtml(doc) {

    var res = JsonLd.newLodgingReservation();

    var nameElem = doc.eval("//td[@class=\"title-hotel\"]")[0];
    res.reservationFor.name = nameElem.content;

    var addrElem = nameElem.parent.nextSibling.nextSibling.nextSibling.firstChild;
    var addr = addrElem.content.match(/(.*), ?([^,]*)/);
    res.reservationFor.address.streetAddress = addr[1];
    res.reservationFor.address.addressLocality = addr[2];
    res.reservationFor.url = lastChild(addrElem).attribute("href");

    var linkRoot = addrElem.parent.nextSibling.nextSibling;
    var links = linkRoot.eval(".//a");
    for (var i = 0; i < links.length; ++i) {
        var url = links[i].attribute("href");
        if (url.startsWith("tel:"))
            res.reservationFor.telephone = url.substr(4);
        else if (url.startsWith("mailto:"))
            res.reservationFor.email = url.substr(7);
    }

    var bookingRef = doc.eval("//table//tr/td/table//tr/td[2]")[0];
    console.log(bookingRef.recursiveContent)
    if (bookingRef.content) {
        res.reservationNumber = bookingRef.content;
        var bookingRow = bookingRef.parent.nextSibling.nextSibling;

        res.underName.name = bookingRow.firstChild.nextSibling.content;
        bookingRow = bookingRow.nextSibling.nextSibling;

        res.underName.email = bookingRow.firstChild.nextSibling.recursiveContent;
        bookingRow = bookingRow.nextSibling.nextSibling.nextSibling;

        res.checkinTime = parseDateTime(bookingRow.firstChild.nextSibling.content);
        bookingRow = bookingRow.nextSibling.nextSibling;
        res.checkoutTime = parseDateTime(bookingRow.firstChild.nextSibling.content);
    } else {
        res.reservationNumber = doc.eval("//td[@class='table-confirmation text-bold']/a")[0].content;

        dts = doc.eval("//td[2]/strong");
        res.checkinTime = parseDateTime(dts[0].content);
        res.checkoutTime = parseDateTime(dts[1].content);
    }
    return res;
}
