/*
    SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

function extractBookingConfirmation(html) {
    const text = html.root.recursiveContent;
    const bookingNo = text.match(/Booking no.\n([A-Z0-9]{6})/)[1];
    let reservations = [];
    let idx = 0;
    while (true) {
        const flight = text.substr(idx).match(/Flight \d+\n +\S+, (.*)\n(?:.*\n)+?(\d\d: *\d\d)\n(.*) \(([A-Z]{3})\) *\n(?:(.*)\n)?(\d\d:\d\d)\n *(\S.*) \(([A-Z]{3})\)\n(?:(.*)\n)?([A-Z0-9]{2}) +(\d{1,4})\n/);
        if (!flight)
            break;
        idx += flight.index + flight[0].length;

        let res = JsonLd.newFlightReservation();
        res.reservationFor.departureTime = JsonLd.toDateTime(flight[1] + ' ' + flight[2], 'dd MMM yyyy hh: mm', 'en');
        res.reservationFor.departureAirport.name = flight[3];
        res.reservationFor.departureAirport.iataCode = flight[4];
        res.reservationFor.departureTerminal = flight[5];
        res.reservationFor.arrivalTime = JsonLd.toDateTime(flight[1] + ' ' + flight[6], 'dd MMM yyyy hh:mm', 'en');
        res.reservationFor.arrivalAirport.name = flight[7];
        res.reservationFor.arrivalAirport.iataCode = flight[8];
        res.reservationFor.arrivalTerminal = flight[9];
        res.reservationFor.airline.iataCode = flight[10];
        res.reservationFor.flightNumber = flight[11];
        res.reservationNumber = bookingNo;
        reservations.push(res);
    }

    // TODO passenger names

    return reservations;
}
