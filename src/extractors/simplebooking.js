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

function parseHtml(doc)
{
    var text = doc.root.recursiveContent;
    var res = JsonLd.newLodgingReservation();

    res.reservationNumber = text.match(/Booking #:\s*(.*?)\n/)[1];
    res.reservationFor.name = doc.eval("//h1")[0].content;

    res.checkinTime = JsonLd.toDateTime(text.match(/Arrival:\s*(.*?)\n/)[1], "dd MMM yyyy", "en");
    res.checkoutTime = JsonLd.toDateTime(text.match(/Departure:\s*(.*?)\n/)[1], "dd MMM yyyy", "en");

    res.reservationFor.geo.latitude = text.match(/Latitude:\s(-?\d+.\d+)/)[1] * 1.0;
    res.reservationFor.geo.longitude = text.match(/Longitude:\s(-?\d+.\d+)/)[1] * 1.0;

    var addr = text.match(/Lodging information[\n\s]+(.*?)\n[\n\s]+(.*?)\n[\n\s]+(.*?)\n[\n\s]+Telephone: (.*?)\n.*\n*\s+Email:\s+(.*?)\n\s+Internet:\s+(.*?)\n/);
    res.reservationFor.address.streetAddress = addr[2];
    res.reservationFor.address.addressLocality = addr[3];
    res.reservationFor.telephone = addr[4];
    res.reservationFor.email = addr[5];
    res.reservationFor.url = addr[6];

    return res;
}
