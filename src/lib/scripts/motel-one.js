/*
  SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
  SPDX-License-Identifier: LGPL-2.0-or-later
*/

function extractPkPass(pass) {
    let res = JsonLd.newLodgingReservation();
    res.reservationNumber = pass.field['booking-nr'].value;
    res.checkinTime = pass.relevantDate.toJSON();
    const depDate = pass.field['booking-date'].value.match(/ - (\d\d\.\d\d\.\d\d)/);
    const depTime = pass.field['back_reservation'].value.match(/Check-out:.* (\d\d:\d\d)/);
    res.checkoutTime = JsonLd.toDateTime(depDate[1] + ' ' + depTime[1], 'dd.MM.yy hh:mm', 'en');
    res.reservationFor.name = pass.field['hotel-name'].value;
    const addr = pass.field['back_hotel_contact'].value.split('\n');
    res.reservationFor.address.streetAddress = addr[1];
    res.reservationFor.address.addressLocality = addr[2];
    res.reservationFor.telephone = addr[4].match(/: (.*)/)[1];
    res.reservationFor.email = addr[3].match(/: (.*)/)[1];
    res.modifyReservationUrl = pass.field['back_booking_link'].value.match(/(https:.*?)>/)[1];
    return res;
}

function extractConfirmation(text) {
    let res = JsonLd.newLodgingReservation();
    const hotel = text.match(/([A-Z]{2}-[A-Z0-9]+)\n(.*)\n(.*\d{4}) *- *(\d.*\d{4})\n/);
    res.reservationNumber = hotel[1];
    res.reservationFor.name = hotel[2];

    const times = text.match(/(\d\d:\d\d).*\n.*(\d\d:\d\d)/);
    res.checkinTime = JsonLd.toDateTime(hotel[3] + times[1], "dd.MM.yyyyhh:mm", "en");
    res.checkoutTime = JsonLd.toDateTime(hotel[4] + times[2], "dd.MM.yyyyhh:mm", "en");

    const addr = text.match(/(.*)\n(.*)\n(.*)\n(\+\d.*)\n(.*@.*)\n/);
    if (addr[1] !== hotel[2])
        return;
    res.reservationFor.address.streetAddress = addr[2];
    res.reservationFor.address.addressLocality = addr[3];
    res.reservationFor.telephone = addr[4];
    res.reservationFor.email = addr[5];

    return res;
}
