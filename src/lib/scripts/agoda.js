/*
    SPDX-FileCopyrightText: 2020 Vitaliy Ry vit@ntcsys.ru
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

function main(content) {
    var res = JsonLd.newLodgingReservation();
    
    const hotelName = content.eval("//*[@id='lblHotelNameData']");
    res.reservationFor.name = hotelName[0].content;
    const localHotelName = content.eval("//*[@id='lblLocalHotelNameData']");
    if (localHotelName.length && localHotelName[0].content != res.reservationFor.name)
        res.reservationFor.name = res.reservationFor.name + " (" + localHotelName[0].content + ")";

    const hotelAddress = content.eval("//*[@id='lblHotelAddressData']");
    const addr = hotelAddress[0].content.match(/^(.*), (\w.*\w),? ([\dA-Z]+)$/);
    res.reservationFor.address.streetAddress = addr[1];
    res.reservationFor.address.addressCountry = addr[2];
    res.reservationFor.address.postalCode = addr[3];
    // TODO consider lblLocalHotelAddressData when present?

    const hotelCoordinates = content.eval("//a[@id='linkMap' or @id='linkDirection']/@href");
    res.reservationFor.geo = JsonLd.toGeoCoordinates(hotelCoordinates[0].content);

    const arrivalDate = content.eval("//*[@id='lblCheckInData' or @id='checkin-date']");
    const arrivalTime = content.eval("//*[@id='lblHotelCheckInData' or @id='checkin-time']");
    const arrivalDt = arrivalDate[0].recursiveContent.match(/(\d.*\d{4})/)[0] + arrivalTime[0].recursiveContent.match(/(\d{2}:\d{2})/)[0];
    res.checkinTime = JsonLd.toDateTime(arrivalDt, "d MMMM yyyyhh:mm", ["ru", "es"]);

    const departureDate = content.eval("//*[@id='lblCheckOutData' or @id='checkout-date']");
    const departureTime = content.eval("//*[@id='lblHotelCheckOutData' or @id='checkout-time']");
    const departureDt = departureDate[0].recursiveContent.match(/(\d.*\d{4})/)[0] + departureTime[0].recursiveContent.match(/(\d{2}:\d{2})/)[0];
    res.checkoutTime = JsonLd.toDateTime(departureDt, "d MMMM yyyyhh:mm", ["ru", "es"]);

    const bookingRef = content.eval("//span[@id='lblBookingIdData']");
    if (bookingRef.length)
        res.reservationNumber = bookingRef[0].content;

    const name = content.eval("//*[@id='lblGuestNameData' or @id='booking-leadguest']");
    if (name.length)
        res.underName.name = name[0].content;

    const phone = content.eval("//*[@id='property-phone']//a/@href");
    if (phone.length)
        res.reservationFor.telephone = phone[0].content.substr(4);

    return res;
}

