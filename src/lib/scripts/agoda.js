/*
    SPDX-FileCopyrightText: 2020 Vitaliy Ry vit@ntcsys.ru
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

function main(content) {
    var res = JsonLd.newLodgingReservation();
    
    var bookingRef = content.eval("//span[@id='lblBookingIdData']");
    var arrivalDate = content.eval("//span[@id='lblCheckInData']");
    var arrivalTime = content.eval("//span[@id='lblHotelCheckInData']");
    var departureDate = content.eval("//span[@id='lblCheckOutData']");
    var departureTime = content.eval("//span[@id='lblHotelCheckOutData']");
    var hotelName = content.eval("//span[@id='lblHotelNameData']");
    var hotelAddress = content.eval("//span[@id='lblHotelAddressData']");
    var hotelCoordinates = content.eval("//a[@id='linkMap']/@href");

    var gpsCoordinates = hotelCoordinates[0].content.match(/(\d+\.\d+,\d+\.\d+)/)[0].split(",");
    var d1 = arrivalDate[0].content.match(/(.*\d{4})/)[0] + arrivalTime[0].content.match(/(\d{2}:\d{2})/)[0];
    var d2 = departureDate[0].content.match(/(.*\d{4})/)[0] + departureTime[0].content.match(/(\d{2}:\d{2})/)[0];
    
    res.reservationNumber = bookingRef[0].content;
    res.reservationFor.name = hotelName[0].content;
    res.reservationFor.address.streetAddress = hotelAddress[0].content;
    res.checkinTime = JsonLd.toDateTime(d1, "dd MMMM yyyyhh:mm", "ru");
    res.checkoutTime = JsonLd.toDateTime(d2, "dd MMMM yyyyhh:mm", "ru");
    res.reservationFor.geo.latitude = gpsCoordinates[0];
    res.reservationFor.geo.longitude = gpsCoordinates[1];
    
    return res;
}

