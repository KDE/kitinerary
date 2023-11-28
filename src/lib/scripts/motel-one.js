/*
  SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
  SPDX-License-Identifier: LGPL-2.0-or-later
*/

function extractPkPass(pass) {
    let res = JsonLd.newLodgingReservation();
    res.reservationNumber = pass.field['booking-nr'].value;
    res.checkinTime = pass.relevantDate;
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
