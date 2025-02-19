/*
    SPDX-FileCopyrightText: 2025 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

function extractReservation(html) {
    let res = JsonLd.newLodgingReservation();
    res.reservationFor.geo = JsonLd.toGeoCoordinates(html.eval('//a[contains(@href, "google")]')[0].attribute("href"));

    const text = html.root.recursiveContent;
    const hotel = text.match(/\n([0-9A-Z]{13})\n(.*)\n(.*)\n(.*)\n(.*)\n/);
    res.reservationNumber = hotel[1];
    res.reservationFor.name = hotel[2];
    res.reservationFor.address.streetAddress = hotel[3];
    res.reservationFor.address.addressLocality = hotel[4];
    res.reservationFor.address.addressCountry = hotel[5];

    const dates = text.match(/\n(\d\d:\d\d).*\n.*\n(\d\d:\d\d).*\n[\s\S]+\n\S+, (\d\d. \S+ \d{4})\n\S+, (\d\d. \S+ \d{4})\n/);
    res.checkinTime = JsonLd.toDateTime(dates[3] + dates[1], "dd. MMMM yyyyhh:mm", "de");
    res.checkoutTime = JsonLd.toDateTime(dates[4] + dates[2], "dd. MMMM yyyyhh:mm", "de");
    return res;
}
