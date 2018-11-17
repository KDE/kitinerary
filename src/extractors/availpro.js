/*
   Copyright (c) 2018 Volker Krause <vkrause@kde.org>

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

function parseHtml(doc) {
    var elems = doc.eval("/html/body/table/tr/td/table/tr/td/table");

    var bookingRef = elems[1].recursiveContent.match(/Reference\s*(\S+)/);
    if (!bookingRef)
        return null;

    var res = JsonLd.newObject("LodgingReservation");
    res.reservationNumber = bookingRef[1];

    var hotelInfo = elems[3].eval(".//table//table");
    var row = hotelInfo[0].firstChild;
    var addr = row.recursiveContent.match(/([^\n]+)\n\s+([^\n]+)\n\s+([^\n]+)/);
    res.reservationFor = JsonLd.newObject("LodgingBusiness");
    res.reservationFor.name = addr[1];
    res.reservationFor.address = JsonLd.newObject("PostalAddress");
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
    res.underName = JsonLd.newObject("Person");
    res.underName.name = row.firstChild.nextSibling.content;
    row = row.nextSibling.nextSibling.nextSibling;

    var checkin = row.recursiveContent.match(/from (\d{1,2}:\d{2})\)\s*(.*)/);
    res.checkinTime = JsonLd.toDateTime(checkin[2] + checkin[1], "dddd, MMMM dd, yyyyhh:mm", "en");
    row = row.nextSibling;
    var checkout = row.recursiveContent.match(/until (\d{1,2}:\d{2})\)\s*(.*)/);
    res.checkoutTime = JsonLd.toDateTime(checkout[2] + checkout[1], "dddd, MMMM dd, yyyyhh:mm", "en");

    return res;
}
