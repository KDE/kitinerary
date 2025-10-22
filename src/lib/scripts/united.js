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
    // Some airports don't parse correctly (e.g. LAS) so we should provide the IATA code if available.
    // Example: "Chicago, IL, US (ORD)" should turn into "ORD"
    const iata = name.match(/.*\((.*)\)/);
    if (iata) {
        airport.iataCode = iata[1];
    }
    return airport;
}

function main(html) {
    var baseReservations = new Array();

    var confirmationNumberElement = html.eval("//tr[td[text()='Confirmation Number:']]/following-sibling::tr[1]")[0]
    if (!confirmationNumberElement)
        return {};

    // Each flight is in it's own table
    var rows = html.eval("//tr/td[not(contains(., 'MileagePlus')) and not(contains(., 'Purchase Summary'))]/table[tr[td[contains(., 'Flight')]]]")
    for (var i in rows) {
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

        baseReservations.push(res);
    }

    // We need to duplicate the baseReservations as needed for each passenger.
    var finalReservations = new Array();

    // Parse the seats, they are in a completely separate thing in the end
    // The first element is the header: "Traveler Details" and then it lists each seat until we hit another name.
    var travelerInformation = html.eval("//tr/td/table[tr[td[contains(., 'Traveler Details')]]]")[1]
    var travelerRow = travelerInformation.firstChild;
    var j = 0;
    var newPassengerReservations;
    while (!travelerRow.isNull) {
        const travelerRowContent = travelerRow.recursiveContent;

        // Names are the only thing that uses slashes here, so its a good indicator.
        if (!travelerRowContent.includes("Traveler Details")) {
            if (travelerRowContent.includes("/")) {
                // Traveler name, the name is usually garbage but it's useful to keep anyway
                // It follows the format FAMILY/GIVEN and all in uppercase, so just deal with that
                const name = travelerRow.recursiveContent.split("/");
                if (name.length == 2) {
                    const familyName = name[0];
                    const givenName = name[1];

                    // Commit the previous passenger, if any.
                    if (newPassengerReservations) {
                        finalReservations = finalReservations.concat(newPassengerReservations);
                        j = 0;
                    }

                    newPassengerReservations = JsonLd.clone(baseReservations);
                    for (var z in newPassengerReservations) {
                        newPassengerReservations[z].underName.familyName = familyName;
                        newPassengerReservations[z].underName.givenName = givenName;
                    }
                }
            } else {
                // The remaining lines are always seat assignments.
                // The first one usually contains the eTicket number.
                // Example: "JFK-IAD 29C" or "eTicket number: 0123456789012 Seats:  AAA-BBB 25E"
                // But a bad example is "Economy Seat Assignment (0123456789012) AAA-BBB" which we need to ignore
                if (!travelerRowContent.includes("Seat Assignment")) {
                    if (travelerRowContent.includes("eTicket number")) {
                        const seats = travelerRowContent.match(/eTicket number: (?:\d*) Seats:  ([A-Z]{3})-([A-Z]{3})\s([0-9]{2}[A-Z])/);
                        newPassengerReservations[j].airplaneSeat = seats[3];
                        j++;
                    } else {
                        const seats = travelerRowContent.match(/([A-Z]{3})-([A-Z]{3})\s([0-9]{2}[A-Z])/);
                        if (seats) {
                            newPassengerReservations[j].airplaneSeat = seats[3];
                            j++;
                        }
                    }
                }
            }
        }

        travelerRow = travelerRow.nextSibling;
    }

    // Commit the last passenger, if any.
    if (newPassengerReservations) {
        finalReservations = finalReservations.concat(newPassengerReservations);
    }

    return finalReservations;
}
