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

    // If dropoff-address is present, it's a one-way rental,
    // otherwise assume the dropoff is identical with pickup.
    // There is no geo coordinates for dropoff provided.
    const dropoffAddress = pass.field["dropoff-address"];
    if (dropoffAddress) {
        const dropoffAddressParts = dropoffAddress.value.split("\n");

        if (!res.dropoffLocation.name) {
            res.dropoffLocation.name = dropoffAddressParts[0];
        }

        res.dropoffLocation.address.streetAddress = dropoffAddressParts[1];

        const dropoffTelLine = dropoffAddressParts[3];
        if (dropoffTelLine && dropoffTelLine.startsWith(telPrefix)) {
            res.dropoffLocation.telephone = dropoffTelLine.substring(telPrefix.length);
        }
    } else {
        res.dropoffLocation = res.pickupLocation;
    }

    res.reservationFor.model = pass.field["vehicle-group-detail"].value;

    // TODO res.reservationFor.rentalCompany.name but Sunny Cars isn't
    // the actual rental car operator. partner-rate-code might help here.

    return [res];
}
