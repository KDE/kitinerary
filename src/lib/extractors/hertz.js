/*
   SPDX-FileCopyrightText: 2018-2019 Laurent Montel <montel@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

function main(text) {
    var res = JsonLd.newObject("RentalCarReservation");

    var bookingRef = text.match(/Your reservation number is\s+([A-Z0-9-]+)\s+/);
    if (!bookingRef)
        return null;
    res.reservationNumber = bookingRef[1];

    var idx = bookingRef.index + bookingRef[0].length;
    res.underName = JsonLd.newObject("Person");
    var name = text.substr(idx).match(/Customer Name:\s+(.*)/);
    if (!name)
        return null;
    res.underName.name = name[1];
    idx += name.index + name[0].length;

    var renting = text.substr(idx).match(/Renting\s+/);
    if (!renting)
        return null;
    idx += renting.index + renting[0].length;
    var cityPickup = text.substr(idx).match(/City:\s+(.*)/);
    res.pickupLocation = JsonLd.newObject("Place");
    res.pickupLocation.address = JsonLd.newObject("PostalAddress");
    res.pickupLocation.address.addressLocality = cityPickup[1];
    idx += cityPickup.index + cityPickup[0].length;

    var locationPickup = text.substr(idx).match(/Location:\s+(.*)/);
    res.pickupLocation.name = locationPickup[1];
    idx += locationPickup.index + locationPickup[0].length;

    var addressPickup = text.substr(idx).match(/Address:\s+(.*)/);
    res.pickupLocation.address.streetAddress = addressPickup[1];
    idx += addressPickup.index + addressPickup[0].length;

    //Problem for parsing date/time !
    var pickUpDate = text.substr(idx).match(/Date\/Time:\s+(.*)/)
    if (!pickUpDate)
        return null;
    res.pickupTime = JsonLd.toDateTime(pickUpDate[1], "ddd dd MMM yyyy hh:mm A", "en");
    idx += pickUpDate.index + pickUpDate[0].length;


    //Add tel/email etc.

    var returnCar = text.substr(idx).match(/Return\s+/);
    if (!returnCar)
        return null;
    idx += returnCar.index + returnCar[0].length;
    var cityDropoff = text.substr(idx).match(/City:\s+(.*)/);
    res.dropoffLocation = JsonLd.newObject("Place");
    res.dropoffLocation.address = JsonLd.newObject("PostalAddress");
    res.dropoffLocation.address.addressLocality = cityDropoff[1];
    idx += cityDropoff.index + cityDropoff[0].length;

    var locationDropOff = text.substr(idx).match(/Location:\s+(.*)/);
    res.dropoffLocation.name = locationDropOff[1];
    idx += locationDropOff.index + locationDropOff[0].length;

    var addressDropOff = text.substr(idx).match(/Address:\s+(.*)/);
    res.dropoffLocation.address.streetAddress = addressDropOff[1];
    idx += addressDropOff.index + addressDropOff[0].length;

    var dropOffDate = text.substr(idx).match(/Date\/Time:\s*(.*)/)
    if (!dropOffDate)
        return null;

    //Need to convert datetime as "Date/Time: MON 22 MAY 2017 09:30 AM"
    res.dropoffTime = JsonLd.toDateTime(dropOffDate[1], "ddd dd MMM yyyy hh:mm A", "en");
    idx += dropOffDate.index + dropOffDate[0].length;

    res.reservationFor = JsonLd.newObject("RentalCar");
    //Fix me it seems to use 2 lines !
    var vehiculeType = text.substr(idx).match(/Vehicle:\s+(.*)\s+/)
    if (!vehiculeType)
        return null;
    res.reservationFor.model = vehiculeType[1];
    idx += vehiculeType.index + vehiculeType[0].length;

    res.reservationFor.rentalCompany = JsonLd.newObject("Organization");
    res.reservationFor.rentalCompany.name = "Hertz"

    return res;
}
