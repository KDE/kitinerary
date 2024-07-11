// SPDX-FileCopyrightText: 2024 Volker Krause <vkrause@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

function extractMail(text) {
    let res = JsonLd.newLodgingReservation();
    res.reservationNumber = text.match(/Reference Number\s+(\S+)\n/)[1];
    const addr = text.match(/(\S.*\S)[\s-]+Address\s*\n(\S.*),\n\s*(\S.*), (.*?), (.*?), (.*?)\n/);
    res.reservationFor.name = addr[1];
    res.reservationFor.address.addressCountry = addr[6];
    res.reservationFor.address.addressRegion = addr[4];
    res.reservationFor.address.postalCode = addr[5];
    res.reservationFor.address.addressLocality = addr[3];
    res.reservationFor.address.streetAddress = addr[2];
    res.reservationFor.telephone = text.match(/Phone\s+(\S+)\n/)[1];
    res.reservationFor.geo = JsonLd.toGeoCoordinates(text.match(/(https:\/\/www.google.*)\n/)[1]);
    res.checkinDate = JsonLd.toDateTime(text.match(/Check In Date\s+(\d{2} \S{3} \d{4})/)[1], "dd MMM yyyy", "en");
    res.checkoutDate = JsonLd.toDateTime(text.match(/Check Out Date\s+(\d{2} \S{3} \d{4})/)[1], "dd MMM yyyy", "en");
    return res;
}
