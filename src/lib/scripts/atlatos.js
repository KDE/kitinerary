/*
   SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
   SPDX-License-Identifier: LGPL-2.0-or-later
*/

// NOTE: times are marked as Europe/Berlin but meant to be local time so we cannot trust that...
function extractEvent(event) {
    let res = JsonLd.newFlightReservation();
    res.underName.name = event.description.match(/^\S+ (.*?),\n/)[1];
    const resNum = event.description.match(/Check-in\): ([A-Z0-9]{2})\/([A-Z0-9]{6})/);
    res.reservationFor.airline.iataCode = resNum[1];
    res.reservationNumber = resNum[2];
    const flight = event.description.match(/Flug: (\S.*\S) (\d{1,4})\n/);
    res.reservationFor.airline.name = flight[1];
    res.reservationFor.flightNumber = flight[2];
    const dep = event.description.match(/ab: (\S.*\S) \(([A-Z]{3})\)\num: (.*) Uhr\n/);
    res.reservationFor.departureAirport.name = dep[1];
    res.reservationFor.departureAirport.iataCode = dep[2];
    res.reservationFor.departureTime = JsonLd.toDateTime(dep[3], 'dd.MM.yyyy - hh:mm', 'de');
    const arr = event.description.match(/an: (\S.*\S) \(([A-Z]{3})\)\num: (.*) Uhr\n/);
    res.reservationFor.arrivalAirport.name = arr[1];
    res.reservationFor.arrivalAirport.iataCode = arr[2];
    res.reservationFor.arrivalTime = JsonLd.toDateTime(arr[3], 'dd.MM.yyyy - hh:mm', 'de');
    return res;
}

function extractPdf(pdf)
{
    // assemble the body text without the extensive footers
    let text = "";
    for (const page of pdf.pages) {
        text += page.textInRect(0.0, 0.0, 1.0, 0.9);
    }

    let reservations = [];
    let idx = 0;
    while (true) {
        const flight = text.substr(idx).match(/: +(\S.*\S) +([A-Z0-9]{2})\/([A-Z0-9]{6})\n.*: +(\S.*?), (\S.*(?:\n +.*)*)\n.*(\d\d[\d\.: ]{14}).*\n.* ([A-Z0-9]{2}) (\d{1,4}) .*\n.*\n.*: +(\S.*?), (\S.*(?:\n +.*)*)\n.*(\d\d[\d\.: ]{14}).*\n/);
        if (!flight) {
            break;
        }
        idx += flight.index + flight[0].length;

        let res = JsonLd.newFlightReservation();
        res.reservationFor.airline.name = flight[1];
        res.reservationFor.airline.iataCode = flight[2];
        res.reservationNumber = flight[3];
        res.reservationFor.departureAirport.address = { '@type': 'PostalAddress', addressLocality: flight[4] };
        res.reservationFor.departureAirport.name = flight[5];
        res.reservationFor.departureTime = JsonLd.toDateTime(flight[6], 'dd.MM.yyyy hh:mm', 'de');
        res.reservationFor.flightNumber = flight[8];
        res.reservationFor.arrivalAirport.address = { '@type': 'PostalAddress', addressLocality: flight[9] };
        res.reservationFor.arrivalAirport.name = flight[10];
        res.reservationFor.arrivalTime = JsonLd.toDateTime(flight[11], 'dd.MM.yyyy hh:mm', 'de');
        reservations.push(res);
    }

    const price = text.match(/Gesamtpreis:(.*)/);
    if (price)
        ExtractorEngine.extractPrice(price[1], reservations);

    return reservations;
}
