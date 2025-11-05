// SPDX-FileCopyrightText: 2025 Volker Krause <vkrause@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

function extractEvent(ev) {
    let res = JsonLd.newFlightReservation();
    res.reservationNumber = ev.description.match(/Checkin reference: (.*)\n/)[1];
    res.underName.name = ev.description.match(/Travelers: (.*)\n/)[1];
    res.reservationFor.departureTime = JsonLd.readQDateTime(ev, "dtStart");
    res.reservationFor.arrivalTime = JsonLd.readQDateTime(ev, "dtEnd");
    res.reservationFor.departureAirport.iataCode = ev.location;
    const arr = ev.summary.match(/Flight to (.*) \((.*)\)/);
    res.reservationFor.arrivalAirport.name = arr[1];
    res.reservationFor.arrivalAirport.iataCode = arr[2];
    ExtractorEngine.extractPrice(ev.description, res);
    return res;
}

function extractHtml(html) {
    const text = html.root.recursiveContent;
    let reservations = [];
    let idx = 0;
    while (true) {
        const flight = text.substr(idx).match(/Flight (.*) \/ ([A-Z0-9]{2})(\d{1,4})\n.* ([A-Z0-9]{6,7})\nFrom (.*) \/ (.*) \(([A-Z]{3})\)( .*)?\nTo (.*) \/ (.*) \(([A-Z]{3})\)( .*)?\n.*(\d{4}-\d{2}-\d{2} \d\d:\d\d)\n.*(\d{4}-\d{2}-\d{2} \d\d:\d\d)\n/);
        if (!flight)
            break;
        idx += flight.index + flight[0].length;

        let res = JsonLd.newFlightReservation();
        res.reservationFor.airline.name = flight[1];
        res.reservationFor.airline.iataCode = flight[2];
        res.reservationFor.flightNumber = flight[3];
        res.reservationNumber = flight[4];
        res.reservationFor.departureAirport.address.addressLocality = flight[5];
        res.reservationFor.departureAirport.name = flight[6];
        res.reservationFor.departureAirport.iataCode = flight[7];
        res.reservationFor.departureTerminal = flight[8];
        res.reservationFor.arrivalAirport.address.addressLocality = flight[9];
        res.reservationFor.arrivalAirport.name = flight[10];
        res.reservationFor.arrivalAirport.iataCode = flight[11];
        res.reservationFor.arrivalTerminal = flight[12];
        res.reservationFor.departureTime = JsonLd.toDateTime(flight[13], "yyyy-MM-dd HH:mm", "en");
        res.reservationFor.arrivalTime = JsonLd.toDateTime(flight[14], "yyyy-MM-dd HH:mm", "en");
        reservations.push(res);
    }
    return reservations;
}
