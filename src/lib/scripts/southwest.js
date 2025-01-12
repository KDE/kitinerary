/*
 *    SPDX-FileCopyrightText: 2025 Joshua Goins <josh@redstrate.com>
 *
 *    SPDX-License-Identifier: LGPL-2.0-or-later
 */

// Example: Wednesday, 01/01/2025 01:00 AM
const southwestDateFormat = "dddd, MM/dd/yyyy hh:mm ap";

function collectChildren(element) {
    var children = new Array();
    var child = element.firstChild;
    while (!child.isNull) {
        children.push(child);
        child = child.nextSibling;
    }
    return children;
}

function parseStop(airport, stop, meridiem, date) {
    const parsed = stop.match(/([A-Z]{3})\s([0-9]{2}:[0-9]{2})/);

    airport.iataCode = parsed[1];
    return JsonLd.toDateTime(date + ' ' + parsed[2] + ' ' + meridiem, southwestDateFormat, "en");
}

function main(html) {
    var reservations = new Array();

    const itinerary = html.eval("//table[contains(@id, 'itn')]")[0];
    if (!itinerary) {
        return {};
    }
    const flightTable = itinerary.eval("tbody/tr/td/table/tbody")[0];
    if (!flightTable) {
        return {};
    }

    const confirmationNumberElement = html.eval("//p[contains(., 'Confirmation #')]/span/strong")[0];
    if (!confirmationNumberElement) {
        return {};
    }

    var currentDate

    const rows = collectChildren(flightTable);
    for (var i in rows) {
        // The "Flight N: ..." row gives us the date that the subsequent flights are based upon
        const newFlightHeader = rows[i].eval("td/table/tbody/tr[contains(., 'Flight')]")[0];
        if (newFlightHeader) {
            const date = newFlightHeader.eval("td/table/tbody/tr/th/table/tbody/tr/td/p")[0];
            currentDate = date.content;
        }

        // The "FLIGHT" row denotes a step in the journey
        const flightHeader = rows[i].eval("td/table/tbody/tr[contains(., 'FLIGHT')]")[0];
        if (flightHeader) {
            let res = JsonLd.newFlightReservation();
            res.reservationNumber = confirmationNumberElement.content;

            const flightNumberElement = flightHeader.eval("td/p")[1];
            const flightNumber = flightNumberElement.content.match(/#\s([0-9]+)/);
            res.reservationFor.flightNumber = flightNumber[1];

            // TODO: Maybe other Southwest tickets put the IATA code for the flights?
            res.reservationFor.airline.iataCode = "WN";
            res.reservationFor.airline.name = "Southwest";

            const stops = flightHeader.eval("td/p/span/strong");
            const departureStop = stops[0].content;
            const arrivalStop = stops[1].content;

            const stopMeridiems = flightHeader.eval("td/p[./span/strong]");
            const departureStopMeridiem = stopMeridiems[0].content;
            const arrivalStopMeridiem = stopMeridiems[1].content;

            res.reservationFor.departureTime = parseStop(res.reservationFor.departureAirport, departureStop, departureStopMeridiem, currentDate);
            res.reservationFor.arrivalTime = parseStop(res.reservationFor.arrivalAirport, arrivalStop, arrivalStopMeridiem, currentDate);

            reservations.push(res);
        }
    }

    return reservations;
}

