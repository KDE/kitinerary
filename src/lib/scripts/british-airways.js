/*
   SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
   SPDX-License-Identifier: LGPL-2.0-or-later
*/

function extractBoardingPass(pdf, node, barcode) {
    const text = pdf.pages[barcode.location].text;
    let res = barcode.result[0];
    const times = text.match(/(\d\d:\d\d) +(\d\d:\d\d)\n/);
    res.reservationFor.boardingTime = JsonLd.toDateTime(times[1], "hh:mm", "en");
    res.reservationFor.departureTime = JsonLd.toDateTime(times[2], "hh:mm", "en");
    return res;
}

function extractConfirmationMail(text) {
    const flights = [];

    const lines = text.split('\n');
    if (text.startsWith('e-ticket receipt')) {
        return [];
    }
    let bookingReference = '';
    for (let i = 0, count = lines.length; i < count; i++) {
        if (lines[i].startsWith('British Airways booking reference: ')) {
            bookingReference = lines[i].split(': ')[1];
        }
        if (lines[i] === 'Your Itinerary') {
            i++;
            while (true) {
                // Iterate over the following structure:
                // ----------------------------------------------------
                // BA0981: British Airways | Euro Traveller | Confirmed
                // ----------------------------------------------------
                // Depart: 28 Mar 2025 06:25 - Brandenburg (Berlin) - Terminal 1
                // Arrive: 28 Mar 2025 07:40 - Heathrow (London) - Terminal 5

                i++;
                i++;
                if (lines[i] === "Passenger list") {
                    break;
                }
                const infoLine = lines[i].split(':'); i++; i++;
                const departureLine = lines[i].substring(lines[i].indexOf(':') + 2).split(' - '); i++;
                const arrivalLine = lines[i].substring(lines[i].indexOf(':') + 2).split(' - '); i++;

                const flight = JsonLd.newFlightReservation();
                flight.reservationId = bookingReference;
                flight.reservationFor.flightNumber = infoLine[0];
                flight.reservationFor.airline.name = infoLine[1].split('|')[0].trim();

                flight.reservationFor.departureTime = JsonLd.toDateTime(departureLine[0], 'dd MMM yyyy hh:mm', 'en');
                flight.reservationFor.departureAirport.name = departureLine[1];
                flight.reservationFor.departureTerminal = departureLine[2].split(' ')[1];

                flight.reservationFor.arrivalTime = JsonLd.toDateTime(arrivalLine[0], 'dd MMM yyyy hh:mm', 'en');
                flight.reservationFor.arrivalAirport.name = arrivalLine[1];
                flight.reservationFor.arrivalTerminal = arrivalLine[2].split(' ')[1];


                flights.push(flight);
            }

            i++;
            i++;

            function toTitleCase(str) {
                return str.replace(/\w\S*/g,
                    text => text.charAt(0).toUpperCase() + text.substring(1).toLowerCase()
                );
            }


            // parsing first name in passenger list
            for (let flight of flights) {
                flight.underName.name = toTitleCase(lines[i]);
            }

        }
    }
    return flights;
}

