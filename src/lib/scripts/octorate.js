/*
   SPDX-FileCopyrightText: 2019-2022 Volker Krause <vkrause@kde.org>
   SPDX-License-Identifier: LGPL-2.0-or-later
*/

function parseEvent(event) {
    let res = JsonLd.newLodgingReservation();
    res.checkinTime = JsonLd.readQDateTime(event, 'dtStart');
    res.checkoutTime = JsonLd.readQDateTime(event, 'dtEnd');
    res.underName.name = event.attendees[0].name;
    res.modifyReservationUrl = event.url;
    res.reservationFor.name = event.organizer.name;
    const loc = event.location.split(', ');
    if (loc.length == 4) {
        res.reservationFor.address.streetAddress = loc[0];
        res.reservationFor.address.addressLocality = loc[2];
        res.reservationFor.address.addressCountry = loc[3];
    }
    if (event.hasGeo) {
        res.reservationFor.geo.latitude = event.geoLatitude;
        res.reservationFor.geo.longitude = event.geoLongitude;
    }
    const ref = event.summary.match(/Reservation ([A-Z0-9]+)$/);
    if (ref) {
        res.reservationNumber = ref[1];
    }
    return res;
}
