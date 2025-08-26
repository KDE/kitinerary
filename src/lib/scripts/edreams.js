// SPDX-FileCopyrightText: 2025 Volker Krause <vkrause@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

function extractConfirmation(html) {
    let res = JsonLd.newLodgingReservation();
    console.log(html);
    const text = html.root.recursiveContent;
    const hotelName = text.match(/(.*)\nPhone: (.*)\nAddress: (.*)\n/);
    res.reservationFor.name = hotelName[1];
    res.reservationFor.telephone = hotelName[2];
    const addr = hotelName[3].split(',');
    res.reservationFor.address.addressCountry = addr[addr.length - 1];
    res.reservationFor.address.addressLocality = addr[addr.length - 2];
    res.reservationFor.address.postalCode = addr[addr.length - 3];
    res.reservationFor.address.streetAddress = addr.slice(0, -3).join(', ');
    const checkin = text.match(/Check-in\n.*(\d{2} \S{3} \d{4}).*(\d\d:\d\d)/);
    res.checkinTime = JsonLd.toDateTime(checkin[1] + ' ' + checkin[2], "dd MMM yyyy HH:mm", "en");
    const checkout = text.match(/Check-out\n.*(\d{2} \S{3} \d{4}).*(\d\d:\d\d)/);
    res.checkoutTime = JsonLd.toDateTime(checkout[1] + ' ' + checkout[2], "dd MMM yyyy HH:mm", "en");
    return res;
}
