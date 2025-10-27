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
    return JsonLd.toDateTime(text.substr(3).trim(), "dd MMM yyyy hh:mm a", "en");
}

function parseAddress(res, addrText)
{
    const addr = addrText.match(/(.*), ?([^,]*)/);
    if (addr) {
        res.reservationFor.address.streetAddress = addr[1];
        res.reservationFor.address.addressLocality = addr[2];
        return true;
    }
    return false;
}

function parseHtml(doc) {
    const text = doc.root.recursiveContent;
    let res = JsonLd.newLodgingReservation();

    let nameElem = doc.eval("//td[@class=\"title-hotel\" or @class=\"hotel-title\"]");
    nameElem = nameElem.length > 0 ? nameElem[0] : undefined;
    if (nameElem) {
        res.reservationFor.name = nameElem.content;
    } else {
        res.reservationFor.name = text.match(/check-in\n(.*)\n/)[1];
    }

    let addrElem = doc.eval("//td[@class=\"hotel-address\"]");
    addrElem = addrElem.length > 0 ? addrElem[0] : undefined;
    if (!addrElem && nameElem) {
        addrElem = nameElem.parent.nextSibling.nextSibling.nextSibling.firstChild;
    }
    if (addrElem) {
        parseAddress(res, addrElem.content) || parseAddress(res, addrElem.recursiveContent);
        res.reservationFor.url = lastChild(addrElem).attribute("href");
    } else {
        parseAddress(res, text.match(/check-in\n.*\n(.*)\n/)[1]);
    }

    var linkRoot = addrElem ? addrElem.parent.parent : doc.root;
    var links = linkRoot.eval(".//a");
    for (var i = 0; i < links.length; ++i) {
        var url = links[i].attribute("href");
        if (url.startsWith("tel:"))
            res.reservationFor.telephone = url.substr(4);
        else if (url.startsWith("mailto:") && !res.reservationFor.email)
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
            res.reservationNumber = text.match(/(?:Reservation number|Reservierungsnummer|reservation number is ):?[\n ]?(\d+)/)[1];
            res.checkinTime = parseDateTime(text.match(/(?:Check-in|Anreise):[\n ](.*)/i)[1]);
            res.checkoutTime = parseDateTime(text.match(/(?:Check-out|Abreise):[\n ](.*)/i)[1]);
        }
    }
    return res;
}
