/*
    SPDX-FileCopyrightText: 2024 Joshua Goins <josh@redstrate.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

// Example: Tue, Sep 03, 2024 02:23 AM
const unitedDateFormat = "ddd, MMM d, yyyy hh:mm ap";

function collectChildren(element) {
    var children = new Array();
    var child = element.firstChild;
    while (!child.isNull) {
        children.push(child);
        child = child.nextSibling;
    }
    return children;
}

function parseAirport(airport, name) {
    airport.name = name;
    return airport;
}

function main(html) {
    var reservations = new Array();
    console.log(html);

    var confirmationNumberElement = html.eval("//tr[td[text()='Confirmation Number:']]/following-sibling::tr[1]")[0]
    if (!confirmationNumberElement)
        return {};

    // Each flight is in it's own table
    var rows = html.eval("//tr/td/table[tr[td[contains(., 'Flight')]]]")
    for (var i in rows) {
        // Deficiency in the XPath, the first table is not relevant
        if (i == 0)
            continue;

        var res = JsonLd.newFlightReservation();
        res.reservationNumber = confirmationNumberElement.recursiveContent;

        // Then the table is made up of four <tr>
        var tableRow = rows[i].firstChild;
        for (var j = 0; j < 4; j++) {
            var departureDate;
            var arrivalDate;

            const children = collectChildren(tableRow);
            if (j == 0) {
                // Flight index, flight number and class
                const flightNumber = children[0].content;
                const flightClass = children[2];

                // Example: Flight 1 of 4 UA1757
                var airline = flightNumber.match(/Flight\s([0-9])\sof\s[0-9]\s([A-Z0-9]{2})(\d{1,4})\b/);
                if (!airline)
                    continue;
                res.reservationFor.airline.iataCode = airline[2];
                // TODO: what to do about other IATA's? I need to find a previous reservation with an "operated by" tag probably.
                if (airline[2] == "UA") {
                    res.reservationFor.airline.name = "United Airlines";
                }
                res.reservationFor.flightNumber = airline[3];
            } else if (j == 1) {
                // Departure and arrival dates
                departureDate = children[0].content;
                arrivalDate = children[2].content;
            } else if (j == 2) {
                // Departure and arrival times
                const departureTime = children[0].content;
                const arrivalTime = children[2].content;

                res.reservationFor.arrivalTime = JsonLd.toDateTime(arrivalDate + ' ' + arrivalTime, unitedDateFormat, "en");
                res.reservationFor.departureTime = JsonLd.toDateTime(departureDate + ' ' + departureTime, unitedDateFormat, "en");
            } else if (j == 3) {
                // Departure and arrival airports
                const departureAirport = children[0].content;
                const arrivalAirport = children[2].content;
                parseAirport(res.reservationFor.departureAirport, departureAirport);
                parseAirport(res.reservationFor.arrivalAirport, arrivalAirport);
            }
            tableRow = tableRow.nextSibling;
        }

        reservations.push(res);
    }

    // Parse the seats, they are in a completely separate thing in the end
    var travelerInformation = html.eval("//tr/td/table[tr[td[contains(., 'Traveler Details')]]]")[1]
    var travelerRow = travelerInformation.firstChild;
    var i = 0;
    var j = 0;
    while (!travelerRow.isNull) {
        if (i == 1) {
            // Traveler name, the name is usually garbage but it's useful to keep anyway
            res.underName = travelerRow.recursiveContent;
        } else if (i > 1) {
            // Possibly contains seating information
            // Example: JFK-IAD 29C
            const seats = travelerRow.recursiveContent.match(/([A-Z]{3})-([A-Z]{3})\s([0-9]{2}[A-Z])/);
            if (seats) {
                // TODO: This is almost certainly always in order, but we should find the specific flight connection from the two given to us via the regex.
                reservations[j].airplaneSeat = seats[3];
                j++;
            }
        }
        travelerRow = travelerRow.nextSibling;
        i++;
    }

    return reservations;
}
