/*
   SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

function parseHtml(doc) {
    var elems = doc.eval("/html/body/table/tr/td/table/tr/td/table");

    var bookingRef = elems[1].recursiveContent.match(/Reference\s*(\S+)/);
    if (!bookingRef)
        return null;

    var res = JsonLd.newLodgingReservation();
    res.reservationNumber = bookingRef[1];

    var hotelInfo = elems[3].eval(".//table//table");
    var row = hotelInfo[0].firstChild;
    var addr = row.recursiveContent.match(/([^\n]+)[\n\s]+([^\n]+)\n\s*([^\n]+)/);
    res.reservationFor.name = addr[1];
    res.reservationFor.address.streetAddress = addr[2];
    res.reservationFor.address.addressLocality = addr[3];
    row = row.nextSibling;
    res.reservationFor.geo = JsonLd.toGeoCoordinates(row.firstChild.firstChild.attribute("href"));

    var links = hotelInfo[1].eval(".//a");
    for (var i = 0; i < links.length; ++i) {
        var url = links[i].attribute("href");
        if (url.startsWith("tel:"))
            res.reservationFor.telephone = url.substr(4);
        else if (url.startsWith("mailto:"))
            res.reservationFor.email = url.substr(7);
        else
            res.reservationFor.url = url;
    }

    var booking = elems[5].firstChild.firstChild.firstChild;
    row = booking.firstChild;
    res.underName.name = row.firstChild.nextSibling.content;
    row = row.nextSibling.nextSibling.nextSibling;

    var checkin = row.recursiveContent.match(/from (\d{1,2}:\d{2})\)\s*(.*)/);
    res.checkinTime = JsonLd.toDateTime(checkin[2] + checkin[1], "dddd, MMMM dd, yyyyhh:mm", "en");
    row = row.nextSibling;
    var checkout = row.recursiveContent.match(/until (\d{1,2}:\d{2})\)\s*(.*)/);
    res.checkoutTime = JsonLd.toDateTime(checkout[2] + checkout[1], "dddd, MMMM dd, yyyyhh:mm", "en");

    return res;
}
