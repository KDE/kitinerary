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
    var dt = text.match(/(\d+\/\d+\/\d+).*?(\d+:\d+)/);
    if (dt)
        return JsonLd.toDateTime(dt[1] + dt[2], "dd/MM/yyyyhh:mm", "en");
    return JsonLd.toDateTime(text.substr(3), "dd MMM yyyy hh:mm a", "en");
}

function parseHtml(doc) {

    var res = JsonLd.newLodgingReservation();

    var nameElem = doc.eval("//td[@class=\"title-hotel\" or @class=\"hotel-title\"]")[0];
    res.reservationFor.name = nameElem.content;

    let addrElem = doc.eval("//td[@class=\"hotel-address\"]");
    if (addrElem.length > 0) {
        addrElem = addrElem[0];
    } else {
        addrElem = nameElem.parent.nextSibling.nextSibling.nextSibling.firstChild;
    }
    let addr = addrElem.content.match(/(.*), ?([^,]*)/);
    if (!addr)
        addr = addrElem.recursiveContent.match(/(.*), ?([^,]*)/);
    res.reservationFor.address.streetAddress = addr[1];
    res.reservationFor.address.addressLocality = addr[2];
    res.reservationFor.url = lastChild(addrElem).attribute("href");

    var linkRoot = addrElem.parent.parent;
    var links = linkRoot.eval(".//a");
    for (var i = 0; i < links.length; ++i) {
        var url = links[i].attribute("href");
        if (url.startsWith("tel:"))
            res.reservationFor.telephone = url.substr(4);
        else if (url.startsWith("mailto:"))
            res.reservationFor.email = url.substr(7);
    }

    var bookingRef = doc.eval("//table//tr/td/table//tr/td[2]")[0];
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
        const resNumElem = doc.eval("//td[@class='table-confirmation text-bold']/a");
        if (resNumElem.length > 0) {
            res.reservationNumber = resNumElem[0].content;

            dts = doc.eval("//td[2]/strong");
            res.checkinTime = parseDateTime(dts[0].content);
            res.checkoutTime = parseDateTime(dts[1].content);
        } else {
            const text = doc.root.recursiveContent;
            res.reservationNumber = text.match(/(?:Reservation number|Reservierungsnummer):?[\n ](\d+)/)[1];
            res.checkinTime = parseDateTime(text.match(/(?:Check-in|Anreise):[\n ](.*)/i)[1]);
            res.checkoutTime = parseDateTime(text.match(/(?:Check-out|Abreise):[\n ](.*)/i)[1]);
        }
    }
    return res;
}
