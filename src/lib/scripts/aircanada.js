/*
 *    SPDX-FileCopyrightText: 2026 Joshua Goins <josh@redstrate.com>
 *
 *    SPDX-License-Identifier: LGPL-2.0-or-later
 */

// Example: Tue 1 Sep, 2024 02:23
const airCanadaDateFormat = "ddd d MMM, yyyy hh:mm";

function parseAirport(airport, name) {
    airport.name = name;
    // Some airports don't parse correctly (e.g. LAS) so we should provide the IATA code if available.
    // Example: "Chicago, IL, US (ORD)" should turn into "ORD"
    const iata = name.match(/.*\((.*)\)/);
    if (iata) {
        airport.iataCode = iata[1];
    }
    return airport;
}

function main(html) {
    var reservations = new Array();

    // Reference number
    const bookingReference = html.eval("//h2[@id='ACBookingReferenceHeading']/following::p")[0]
    const bookingReferenceContent = bookingReference.recursiveContent;

    // Flight legs
    const rows = html.eval("//td[@id='ACItineraryInfoContent']/../..");
    for (const i in rows) {
        const infoHeader = rows[i].eval("tr//td[@id='ACItineraryInfoHeader']")[0];

        // Formatted like "Mon 1 Jan, 2026"
        const date = infoHeader.eval("h3/span")[0].recursiveContent;

        const segmentInfoContainer = rows[i].eval("tr//tr[@class='ac-single-segment-info-container']")[0];
        const infoParagraphs = segmentInfoContainer.eval("td//p");

        // Times are formatted in 24h
        const arrivalAirport = infoParagraphs[4].recursiveContent;
        const arrivalTime = infoParagraphs[5].recursiveContent;

        const departureAirport = infoParagraphs[0].recursiveContent;
        const departureTime = infoParagraphs[1].recursiveContent;

        const plane = segmentInfoContainer.eval("td/table/tr/td/table/tr/td")[1].recursiveContent;

        var res = JsonLd.newFlightReservation();
        res.reservationNumber = bookingReferenceContent;

        res.reservationFor.arrivalTime = JsonLd.toDateTime(date + ' ' + arrivalTime, airCanadaDateFormat, "en");
        res.reservationFor.departureTime = JsonLd.toDateTime(date + ' ' + departureTime, airCanadaDateFormat, "en");

        parseAirport(res.reservationFor.departureAirport, departureAirport);
        parseAirport(res.reservationFor.arrivalAirport, arrivalAirport);

        const airline = plane.match(/([A-Z0-9]{2})(\d{1,4})\b/);
        res.reservationFor.airline.iataCode = airline[1];
        // TODO: read from the "Operated By:" tag
        if (airline[1] == "AC") {
            res.reservationFor.airline.name = "Air Canada";
        }
        res.reservationFor.flightNumber = airline[2];

        reservations.push(res);
    }

    // Passenger information
    // TODO: Parse multiple passengers when we encounter those
    var passengerRow = html.eval("//td[@id='ACPassengerInfoContent']/table")[0];
    for (const i in reservations) {
        // Remove all of the whitespace and weird indentation
        const paddedName = passengerRow.eval("tr/td/table/tr/td/h3")[0].recursiveContent;
        const name = paddedName.match(/([\S]*)\s*([\S]*)/);
        reservations[i].underName.familyName = name[2];
        reservations[i].underName.givenName = name[1];
    }

    return reservations;
}
