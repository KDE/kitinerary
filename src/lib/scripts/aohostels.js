/*
   SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

function main(text) {
    var res = JsonLd.newLodgingReservation();

    var bookingRef = text.match(/(?:Booking no\.|Buchungs-Nr\.)\s+([A-Z0-9-]+)\s+/);
    if (!bookingRef)
        return null;
    res.reservationNumber = bookingRef[1];
    var idx = bookingRef.index + bookingRef[0].length;

    var arrivalDate = text.substr(idx).match(/(?:Arrival|Anreise)\s+(\d{1,2}\.\d{1,2}\.\d{4})\s+/)
    if (!arrivalDate)
        return null;
    res.checkinTime = JsonLd.toDateTime(arrivalDate[1] + " 15:00", "dd.MM.yyyy hh:mm", "en");
    idx += arrivalDate.index + arrivalDate[0].length;

    var departureDate = text.substr(idx).match(/(?:Departure|Abreise)\s+(\d{1,2}\.\d{1,2}\.\d{4})\s+/)
    if (!departureDate)
        return null;
    res.checkoutTime = JsonLd.toDateTime(departureDate[1] + " 10:00", "dd.MM.yyyy hh:mm", "en");
    idx += departureDate.index + departureDate[0].length;

    var geo = text.substr(idx).match(/<(http[^>]*google.com\/maps[^>]*)>/);
    res.reservationFor.geo = JsonLd.toGeoCoordinates(geo[1]);

    var name = text.substr(idx).match(/\*(?:First name|Vorname)\*\s+([^\s{2}]+)\s{2,}/);
    if (!name)
        return null;
    res.underName.name = name[1];
    idx += name.index + name[0].length;
    name = text.substr(idx).match(/\*(?:Last name|Nachname)\*\s+([^\s{2}]+)\s{2,}/);
    if (!name)
        return null;
    res.underName.name += ' ' + name[1];
    idx += name.index + name[0].length;

    var hotel = text.substr(idx).match(/(?:Your booked house|Ihr gebuchtes Haus)\s+/);
    if (!hotel)
        return null;
    idx += hotel.index + hotel[0].length;
    hotel = text.substr(idx).split(/\s{2,}/);
    res.reservationFor.name = hotel[0];
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

function extractPass(pass, node) {
    let res = JsonLd.newLodgingReservation();
    res.reservedTicket = node.result[0].reservedTicket;
    res.reservationNumber = pass.field['bookingref'].value;
    res.underName.name = pass.field['passenger'].value;
    res.reservationFor.name = pass.field['destination'].value;
    res.checkinTime = pass.field['arrival'].value;
    res.checkoutTime = pass.field['departure'].value;
    res.reservationFor.address.addressLocality = pass.field['hotel-city'].value;
    res.reservationFor.address.streetAddress = pass.field['address-of-hotel'].value;
    res.reservationFor.telephone = pass.field['phone-of-hotel'].value;
    res.reservationFor.email = pass.field['email-of-hotel'].value;
    return res;
}
