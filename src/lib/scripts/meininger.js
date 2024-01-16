/*
   SPDX-FileCopyrightText: 2024 Volker Krause <vkrause@kde.org>
   SPDX-License-Identifier: LGPL-2.0-or-later
*/

function extractHtml(html) {
    const text = html.root.recursiveContent;
    let res = JsonLd.newLodgingReservation();
    const hotel = text.match(/YOUR HOTEL\n(.*)\n(.*)\n(.*), (.*)\n.*(\d{1,2}:\d{2} .*)\n.*(\d{1,2}:\d{2} .*)\n/);
    res.reservationFor.name = hotel[1];
    res.reservationFor.address.streetAddress = hotel[2];
    res.reservationFor.address.addressLocality = hotel[3];
    res.reservationFor.address.addressCountry = hotel[4];

    const arr = text.match(/Arrival\s*: (.*)\n/)[1];
    const dep = text.match(/Departure\s*: (.*)\n/)[1];
    res.checkinTime = JsonLd.toDateTime(arr + ' ' + hotel[5].replace(/\./g, ''), "dd.MM.yyyy h:mm ap", "en");
    res.checkoutTime = JsonLd.toDateTime(dep + ' ' + hotel[6].replace(/\./g, ''), "dd.MM.yyyy h:mm ap", "en");

    res.reservationNumber = text.match(/Confirmation.*: (.*)\n/)[1];
    res.underName.name = text.match(/Name\s*: (.*)\n/)[1];
    return res;
}
