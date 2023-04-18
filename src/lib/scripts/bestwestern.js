/*
   SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
   SPDX-License-Identifier: LGPL-2.0-or-later
*/

function parseConfirmation(html) {
    let res = JsonLd.newLodgingReservation();
    const text = html.root.recursiveContent;
    const addr = text.match(/Hoteladresse:\n(.*)\n(.*)\n(.*)\n\s+(.*)\n/);
    res.reservationFor.name = addr[1];
    res.reservationFor.address.streetAddress = addr[2];
    res.reservationFor.address.postalCode = addr[3];
    res.reservationFor.address.addressLocality = addr[4];
    const dt = text.match(/Anreise\/Abreise: .* (\d\d\.\d\d\.\d{4})\n\s+\/\s*\n.* (\d\d\.\d\d\.\d{4})/);
    res.checkinTime = JsonLd.toDateTime(dt[1], "dd.MM.yyyy", "de");
    res.checkoutTime = JsonLd.toDateTime(dt[2], "dd.MM.yyyy", "de");
    res.reservationNumber = text.match(/Buchungsnummer: (.*)\n/)[1];
    res.cancelReservationUrl = html.eval('//a[contains(@href,"showBookingCancelPage")]')[0].attribute('href');
    return res;
}
