// SPDX-FileCopyrightText: 2025 Volker Krause <vkrause@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

function extractConfirmation(html) {
    reservations = [];

    const root = html.root.eval('//td[starts-with(@class, "itineraryWrapper")]')[0];
    const headers = root.eval('.//td[contains(@class, " segmentHeaderMarkup")]');

    for (header of headers) {
        const date = header.recursiveContent.match(/.* (\d.*)/)[1];

        const deps = header.eval('../..//td[@class="itineraryDepartureSectionLeg"]');
        const arrs = header.eval('../..//td[@class="itineraryArrivalSectionLeg"]');

        for (let i = 0; i < deps.length; ++i) {
            let res = JsonLd.newFlightReservation();
            const dep = deps[i].recursiveContent.match(/(\d\d:\d\d)\n(.*) \((.*)\)\n([A-Z]{3}) · (.*)\n(.*) · ([A-Z0-9]{2}) (\d{1,4})\n.* ([A-Z0-9]{6,7})\n/);
            res.reservationFor.departureTime = JsonLd.toDateTime(date + ' ' + dep[1], "d MMMM hh:mm", "de");
            res.reservationFor.departureAirport.address.addressLocality = dep[2];
            res.reservationFor.departureAirport.address.addressCountry = dep[3];
            res.reservationFor.departureAirport.iataCode = dep[4];
            res.reservationFor.departureAirport.name = dep[5];
            res.reservationFor.airline.name = dep[6];
            res.reservationFor.airline.iataCode = dep[7];
            res.reservationFor.flightNumber = dep[8];

            const arr = arrs[i].recursiveContent.match(/(\d\d:\d\d)\n(.*) \((.*)\)\n([A-Z]{3}) · (.*)/);
            res.reservationFor.arrivalTime = JsonLd.toDateTime(date + ' ' + arr[1], "d MMMM hh:mm", "de");
            res.reservationFor.arrivalAirport.address.addressLocality = arr[2];
            res.reservationFor.arrivalAirport.address.addressCountry = arr[3];
            res.reservationFor.arrivalAirport.iataCode = arr[4];
            res.reservationFor.arrivalAirport.name = arr[5];

            reservations.push(res);
        }

    }

    return reservations;
}
