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

function extractItinerary(pdf) {
    const text = pdf.text;
    let reservations = [];
    let idx = 0;
    while (true) {
        const leg = text.substr(idx).match(/(\d\d:\d\d) +(\S.*)\n *(\d\d \S{3}) +(\S.*)\n+ +(\S.*\S) *, *([A-Z0-9]{2}) (\d{1,4})\n.*\n.*\n+ *(\d\d:\d\d) +(\S.*)\n *(\d\d \S{3}) +(\S.*)\n/);
        if (!leg)
            break;
        idx += leg.index + leg[0].length;
        let res = JsonLd.newFlightReservation();
        res.reservationFor.departureTime = JsonLd.toDateTime(leg[1] + ' ' + leg[3], 'HH:mm dd MMM', 'en');
        res.reservationFor.departureAirport.address.addressLocality = leg[2];
        res.reservationFor.departureAirport.name = leg[4].replace(/ ,(?=\S)/, ", ");
        res.reservationFor.airline.name = leg[5];
        res.reservationFor.airline.iataCode = leg[6];
        res.reservationFor.flightNumber = leg[7];
        res.reservationFor.arrivalTime = JsonLd.toDateTime(leg[8] + ' ' + leg[10], 'HH:mm dd MMM', 'en');
        res.reservationFor.arrivalAirport.address.addressLocality = leg[9];
        res.reservationFor.arrivalAirport.name = leg[11].replace(/ ,(?=\S)/, ", ");
        reservations.push(res);
    }
    return reservations;
}
