// SPDX-FileCopyrightText: 2025 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: LGPL-2.0-or-later

function parseReservationMail(content) {
    let res = JsonLd.newLodgingReservation();
    res.reservationNumber = content.match(/(?:Reference Number) ?: +([-A-Z0-9]{15})/)[1];

    res.checkinTime = JsonLd.toDateTime(content.match(/(?:Check In) ?: +(.*)/)[1], ["ddd d'th' MMM yyyy", "ddd d'rd' MMM yyyy", "ddd d'st' MMM yyyy"], "en");
    res.checkoutTime = JsonLd.toDateTime(content.match(/(?:Check Out) ?: +(.*)/)[1], ["ddd d'th' MMM yyyy", "ddd d'rd' MMM yyyy", "ddd d'st' MMM yyyy"], "en");

    res.totalPrice = content.match(/(?:Total Cost) ?: +(.*)/)[1]

    const lines = content.split('\n');
    for (let i = 0, count = lines.length; i < count; i++) {
        const line = lines[i];
        if (line.startsWith('Booking Information')) {
            res.reservationFor.name = lines[i + 1];
            const address = lines[i + 2].split(',');
            res.reservationFor.address.streetAddress = address[0];
            res.reservationFor.address.addressLocality = address[1];
            res.reservationFor.address.addressCountry = address[2];

        }
    }

    return res;
}
