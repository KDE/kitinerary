/*
  SPDX-FileCopyrightText: 2024 Volker Krause <vkrause@kde.org>
  SPDX-License-Identifier: LGPL-2.0-or-later
*/

function extractEvent(ev) {
    let res = JsonLd.newLodgingReservation();
    res.checkinTime = ev.dtStart;
    res.checkoutTime = ev.dtEnd;
    const subject = ev.summary.match(/(.*) - ([^,\n]*)(?:, ([A-Z\d]+))?/);
    res.underName.name = subject[1];
    res.reservationFor.name = subject[2];
    res.reservationNumber = subject[3];
    res.reservationFor.address.addressLocality = ev.location;
    const desc = ev.description.match(/Hotel \(.*\)\n.*\n(.*)\n.*\n(.*)\n\n.*: (.*)\n/);
    res.reservationFor.address.streetAddress = desc[1];
    res.reservationFor.address.postalCode = desc[2];
    res.reservationFor.telephone = '+' + desc[3];
    return res;
}
