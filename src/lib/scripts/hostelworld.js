// SPDX-FileCopyrightText: 2025 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: LGPL-2.0-or-later

function parseReservationMail(content) {
    let res = JsonLd.newLodgingReservation();
    res.reservationNumber = content.match(/(?:Reference Number|Reservierungsnummer) ?: +([-A-Z0-9]{15})/)[1];

    res.checkinTime = JsonLd.toDateTime(content.match(/(?:Check In|Einchecken) ?: +[^ ]+ +(.*)/)[1], ["d'th' MMM yyyy", "d'rd' MMM yyyy", "d'st' MMM yyyy", "d MMM yyyy"], ["en","de"]);
    res.checkoutTime = JsonLd.toDateTime(content.match(/(?:Check Out|Auschecken) ?: +[^ ]+ +(.*)/)[1], ["d'th' MMM yyyy", "d'rd' MMM yyyy", "d'st' MMM yyyy", "d MMM yyyy"], ["en","de"]);

    res.totalPrice = content.match(/(?:Total Cost|Gesamtkosten) ?: +(.*)/)[1]

    const lines = content.split('\n');
    for (let i = 0, count = lines.length; i < count; i++) {
        const line = lines[i];
        if (line.startsWith('Booking Information') || line.startsWith('Buchungsinformationen')) {
            res.reservationFor.name = lines[i + 1];
            const address = lines[i + 2].split(',');
            res.reservationFor.address.streetAddress = address[0];
            res.reservationFor.address.addressLocality = address[1];
            res.reservationFor.address.addressCountry = address[2];

        }
    }

    return res;
}

function parseReservationMailV2(content) {
    let res = JsonLd.newLodgingReservation();

    const lines = content.split('\n');
    let dateStart;
    let dateEnd;
    for (let i = 0, count = lines.length; i < count; i++) {
        const line = lines[i];

        if (line.startsWith('Booking Reference')) {
            res.reservationNumber = lines[i + 1];

            const name = lines[i + 2].split(',');
            res.reservationFor.name = name[0]

            const address = lines[i + 3].split(',');
            res.reservationFor.address.streetAddress = address[0];
            res.reservationFor.address.addressLocality = address[address.length - 2];
            res.reservationFor.address.addressCountry = address[address.length - 1];

            const dates = lines[i + 4].split(',')[0].split(' - ');

            dateStart = dates[0]
            dateEnd = dates[1]
        }

        if (line.startsWith('Total')) {
            const match = line.match("Total ([A-Z]{3}) (.*)");
            if (match) {
                res.totalPrice = match[2];
                res.priceCurrency = match[1];
            }
        }

        if (line.startsWith('Check-In:')) {
            const match = line.match("From ([0-9]{2}:[0-9]{2})");
            if (match) {
                dateStart += ' ' + match[1];
            }
        }

        if (line.startsWith('Check-Out:')) {
            const match = line.match("Before ([0-9]{2}:[0-9]{2})");
            if (match) {
                dateEnd += ' ' + match[1];
            }
        }
    }

    res.checkinTime = JsonLd.toDateTime(dateStart, ["d MMM yyyy hh:mm", "d MMM yyyy"], ["en","de"]);
    res.checkoutTime = JsonLd.toDateTime(dateEnd, ["d MMM yyyy hh:mm", "d MMM yyyy"], ["en","de"]);

    return res;
}
