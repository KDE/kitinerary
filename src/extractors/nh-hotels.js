/*
   Copyright (c) 2019 Volker Krause <vkrause@kde.org>

   This library is free software; you can redistribute it and/or modify it
   under the terms of the GNU Library General Public License as published by
   the Free Software Foundation; either version 2 of the License, or (at your
   option) any later version.

   This library is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
   FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
   License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to the
   Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
   02110-1301, USA.
*/

function lastChild(elem) {
    var child = elem.firstChild;
    while (!child.nextSibling.isNull) {
        child = child.nextSibling;
    }
    return child;
}

function parseHtml(doc) {

    var res = JsonLd.newObject("LodgingReservation");
    res.reservationFor = JsonLd.newObject("LodgingBusiness");

    var nameElem = doc.eval("//td[@class=\"title-hotel\"]")[0];
    res.reservationFor.name = nameElem.content;

    var addrElem = nameElem.parent.nextSibling.nextSibling.nextSibling.firstChild;
    var addr = addrElem.content.match(/(.*), ?([^,]*)/);
    res.reservationFor.address = JsonLd.newObject("PostalAddress");
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
    res.reservationNumber = bookingRef.content;
    var bookingRow = bookingRef.parent.nextSibling.nextSibling;

    res.underName = JsonLd.newObject("Person");
    res.underName.name = bookingRow.firstChild.nextSibling.content;
    bookingRow = bookingRow.nextSibling.nextSibling;

    res.underName.email = bookingRow.firstChild.nextSibling.recursiveContent;
    bookingRow = bookingRow.nextSibling.nextSibling.nextSibling;

    var dt = bookingRow.firstChild.nextSibling.content.match(/([\d/]+).*?([\d:]+)/);
    res.checkinTime = JsonLd.toDateTime(dt[1] + dt[2], "dd/MM/yyyyhh:mm", "en");
    bookingRow = bookingRow.nextSibling.nextSibling;

    dt = bookingRow.firstChild.nextSibling.content.match(/([\d/]+).*?([\d:]+)/);
    res.checkoutTime = JsonLd.toDateTime(dt[1] + dt[2], "dd/MM/yyyyhh:mm", "en");

    return res;
}
