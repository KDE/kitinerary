/*
 *    SPDX-FileCopyrightText: 2025 Joshua Goins <josh@redstrate.com>
 *
 *    SPDX-License-Identifier: LGPL-2.0-or-later
 */

// Example: Wednesday, January 01, 2025 01:00 AM
const americanDateFormat = "dddd, MMMM d, yyyy h:mm ap";

function parseAirport(airport, name, label) {
    airport.iataCode = name;
    return airport;
}

function collectChildren(element) {
    var children = new Array();
    var child = element.firstChild;
    while (!child.isNull) {
        children.push(child);
        child = child.nextSibling;
    }
    return children;
}

function main(html) {
    var reservations = new Array();

    // The structure of the American Airlines reservation confirmation are a tangled mess of nested tables
    const itinerary = html.eval("//table[thead[tr[td[contains(., 'Confirmation code:')]]]]")[0]
    const children = collectChildren(itinerary);

    var confirmationNumberElement

    for (const i in children) {
        const tableIndex = Number(i);
        const child = children[tableIndex];
        if (tableIndex === 0) {
            // The first child is the confirmation number
            confirmationNumberElement = child.eval("tr/td/table/tbody/tr/td/span[text()='Confirmation code: ']/following-sibling::span")[0]
            if (!confirmationNumberElement) {
                return {};
            }
        } else if ((tableIndex + 1) < children.length) {
            // Then each subsequent child represents a flight, except for the last which is the "Manage reservation link"
            const subChildren = collectChildren(child);

            // The first child is the date
            const dateElement = subChildren[0].eval("td/table/tbody/tr/td/span")[0];
            if (!dateElement) {
                return {};
            }

            var res = JsonLd.newFlightReservation();
            res.reservationNumber = confirmationNumberElement.recursiveContent;

            // The second child is the arrival/deparature information
            const flightData = subChildren[1].eval("td/table/tr/td/table/tr");
            for (const z in flightData) {
                const row = Number(z);
                const departure = row === 0;
                const arrival = row === 1;

                const flightPaths = collectChildren(flightData[row]);
                for (const j in flightPaths) {
                    const column = Number(j);
                    if (column === 0) {
                        const airportInfo = flightPaths[column].eval("table/tbody/tr/td");

                        const airportCode = airportInfo[0].content;
                        const time = airportInfo[2].content;

                        const parsedDateTime = JsonLd.toDateTime(dateElement.content + ' ' + time, americanDateFormat, "en");
                        if (departure) {
                            parseAirport(res.reservationFor.departureAirport, airportCode);
                            res.reservationFor.departureTime = parsedDateTime;
                        } else if (arrival) {
                            parseAirport(res.reservationFor.arrivalAirport, airportCode);
                            res.reservationFor.arrivalTime = parsedDateTime;
                        }
                    } else if (column === 1) {
                        if (departure) {
                            // The departure row contains the operating airline, and plane information
                            const planeNameElement = flightPaths[column].eval("table/tr/td/table/tr/td/span")[0];

                            const airline = planeNameElement.content.match(/([A-Z]{2})\s(\d{1,4})\b/);
                            if (!airline) {
                                continue;
                            }

                            // TODO: what to do about other IATA's? I need to find a previous reservation with an "operated by" tag probably.
                            if (airline[1] == "AA") {
                                res.reservationFor.airline.name = "American Airlines";
                            }

                            res.reservationFor.flightNumber = airline[2];

                            // TODO: Extract which company operates this airplane, by which also exists in this row
                        } else if (arrival) {
                            // The arrival row contains seat information
                            // TODO: extract seat information, which needs traveler information parsing first
                        }
                    }
                }
            }

            reservations.push(res);
        }
    }

    // TODO: parse traveler information from the "Your purchase" section of the email

    return reservations;
}
