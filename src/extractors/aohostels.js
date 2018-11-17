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

function main(text) {
    var res = JsonLd.newObject("LodgingReservation");

    var bookingRef = text.match(/Booking no\.\s+([A-Z0-9-]+)\s+/);
    if (!bookingRef)
        return null;
    res.reservationNumber = bookingRef[1];
    var idx = bookingRef.index + bookingRef[0].length;

    var arrivalDate = text.substr(idx).match(/Arrival\s+(\d{1,2}\.\d{1,2}\.\d{4})\s+/)
    if (!arrivalDate)
        return null;
    res.checkinTime = JsonLd.toDateTime(arrivalDate[1] + " 15:00", "dd.MM.yyyy hh:mm", "en");
    idx += arrivalDate.index + arrivalDate[0].length;

    var departureDate = text.substr(idx).match(/Departure\s+(\d{1,2}\.\d{1,2}\.\d{4})\s+/)
    if (!departureDate)
        return null;
    res.checkoutTime = JsonLd.toDateTime(departureDate[1] + " 10:00", "dd.MM.yyyy hh:mm", "en");
    idx += departureDate.index + departureDate[0].length;

    res.reservationFor = JsonLd.newObject("LodgingBusiness");
    var geo = text.substr(idx).match(/<(http[^>]*google.com\/maps[^>]*)>/);
    res.reservationFor.geo = JsonLd.toGeoCoordinates(geo[1]);

    res.underName = JsonLd.newObject("Person");
    var name = text.substr(idx).match(/\*First name\*\s+([^\s{2}]+)\s{2,}/);
    if (!name)
        return null;
    res.underName.name = name[1];
    idx += name.index + name[0].length;
    name = text.substr(idx).match(/\*Last name\*\s+([^\s{2}]+)\s{2,}/);
    if (!name)
        return null;
    res.underName.name += ' ' + name[1];
    idx += name.index + name[0].length;

    var hotel = text.substr(idx).match(/Your booked house\s+/);
    if (!hotel)
        return null;
    idx += hotel.index + hotel[0].length;
    hotel = text.substr(idx).split(/\s{2,}/);
    res.reservationFor.name = hotel[0];
    res.reservationFor.address = JsonLd.newObject("PostalAddress");
    res.reservationFor.address.streetAddress = hotel[1];
    var city = hotel[2].match(/(\d+)\s(.*)/);
    if (city) {
        res.reservationFor.address.postalCode = city[1];
        res.reservationFor.address.addressLocality = city[2];
    } else {
        res.reservationFor.address.addressLocality = hotel[2];
    }
    res.reservationFor.address.addressCountry = hotel[3];
    res.reservationFor.telephone = hotel[4];
    res.reservationFor.email = hotel[5];

    return res;
}
