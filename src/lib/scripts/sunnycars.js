/*
   SPDX-FileCopyrightText: 2023 Kai Uwe Broulik <kde@broulik.de>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

function main(pass, node)
{
    let res = JsonLd.newRentalCarReservation();

    res.reservationNumber = pass.serialNumber;

    res.underName.name = pass.field["driver-name"].value;

    res.pickupTime = pass.relevantDate;
    res.dropoffTime = pass.expirationDate;
    // TODO also consider pickup-datetime.value and dropoff-datetime.value for dates.

    res.pickupLocation.name = pass.field["pickup-info"].value;

    const pickupAddress = pass.field["pickup-address"].value;
    const pickupAddressParts = pickupAddress.split("\n");

    if (!res.pickupLocation.name) {
        res.pickupLocation.name = pickupAddressParts[0];
    }

    res.pickupLocation.address.streetAddress = pickupAddressParts[1];

    const telPrefix = "Tel: ";
    if (pickupAddressParts[3].startsWith(telPrefix)) {
        res.pickupLocation.telephone = pickupAddressParts[3].substring(telPrefix.length);
    }
    // TODO pickup-meeting-details

    const location = pass.locations[0];
    res.pickupLocation.geo.latitude = location.latitude;
    res.pickupLocation.geo.longitude = location.longitude;

    // TODO drop-off location. We currently do not have enough samples
    // to tell whether the absence of a drop-off location indicates
    // that it is identical to the pickup location.

    res.reservationFor.model = pass.field["vehicle-group-detail"].value;

    // TODO res.reservationFor.rentalCompany.name but Sunny Cars isn't
    // the actual rental car operator. partner-rate-code might help here.

    return [res];
}
