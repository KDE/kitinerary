/*
 *   SPDX-FileCopyrightText: 2025 Grzegorz (grzesjam) M
 *   SPDX-License-Identifier: LGPL-2.0-or-later
 */

function removeEmptyElements(arr) {
    return arr.filter(element => element !== undefined && element !== null && element !== '').filter(Boolean);
}

function getLanguageConfig(language) {
    const configs = {
        pl: {
            wordToIdentifyRoutes: "odcinek",
            dateTimeFormat: 'hh:mmdd.MM.yyyy',
            parseDirection: (line, routeCount) => routeCount === 1 ? line : line.split(': ')[1],
            parteTrainInfoAndClass: (line) => line.split(' klasa '),
            ticketTitle: /BILET NR/,
        },
        en: {
            wordToIdentifyRoutes: "route",
            dateTimeFormat: 'hh:mmdd.MM.yyyy',
            parseDirection: (line, routeCount) => routeCount === 1 ? line : line.split('route')[1],
            parteTrainInfoAndClass: (line) => removeEmptyElements(line.split(' class')[0].split(' ')),
            ticketTitle: /TICKET NO/,
        },
    };

    return configs[language] || configs['pl'];
}

function main(content, node) {
    let language = "?"
    if (content.text.includes("Bilet ważny wyłącznie w pociągach Spółki PKP Intercity")){
        language = "pl"
    } else if (content.text.includes("The ticket is valid only on PKP Intercity trains")){
        language = "en"
    }


    const config = getLanguageConfig(language);
    const reservations = [];

    // Clean and split lines
    const lines = content.text.replace(/ +/g, ' ').trim().split('\n');

    // Determine the amount routes and phrases to find its array index
    const regex = new RegExp(`\\b(${config.wordToIdentifyRoutes})(\\w*)?\\b`, 'gi');
    const matches = lines.filter(str => regex.test(str)) || []

    // If no matches, it means there is only one route
    const routeCount = matches.length || 1;

    console.log(`Number of routes: ${routeCount}`);

    for (let i = 1; i <= routeCount; i++) {
        const reservation = JsonLd.newTrainReservation();

        const index = lines.indexOf(matches[i-1]);
        const baseIndex = index !== -1 ? index : 7; // If there is only one route its always on 7th index
        console.log(`Processing route ${i}, baseIndex: ${baseIndex}`);

        // Set departure and arrival station names
        const direction = config.parseDirection(lines[baseIndex], routeCount);
        reservation.reservationFor.departureStation.name = direction;
        reservation.reservationFor.arrivalStation.name = direction;

        // Set train name and seating type
        const trainInfo = config.parteTrainInfoAndClass(lines[baseIndex + 3])
        reservation.reservationFor.trainName = trainInfo[0];
        reservation.reservedTicket.ticketedSeat.seatingType = trainInfo[1];

        // Set Departure time, Train number, and Seat section
        const routeDetails = removeEmptyElements(lines[baseIndex + 2].split(' '));
        console.log (routeDetails)
        reservation.reservationFor.departureTime = JsonLd.toDateTime(
            routeDetails[0] + routeDetails[1],
            config.dateTimeFormat,
            language
        );
        reservation.reservationFor.trainNumber = routeDetails[4];
        reservation.reservedTicket.ticketedSeat.seatSection = routeDetails[5];

        // Set arrival time and seat number
        const arrivalDetails = removeEmptyElements(lines[baseIndex + 5].split(' '));
        reservation.reservationFor.arrivalTime = JsonLd.toDateTime(
            arrivalDetails[0] + arrivalDetails[1],
            config.dateTimeFormat,
            language
        );
        reservation.reservedTicket.ticketedSeat.seatNumber = arrivalDetails.slice(2).join(' ');

        // Set static information
        reservation.reservedTicket.underName = lines[1];
        reservation.reservationNumber = lines[6].split(config.ticketTitle)[1];
        reservation.reservedTicket.price = lines[5];
        reservation.reservedTicket.totalPrice = lines[5];
        reservation.reservationProvider = "PKP Intercity"; // It is generated only by PKP IC app (nothing else could be there)

        // Generate ticket token
        reservation.reservedTicket.ticketToken = 'azteccode:' + node.childNodes[1].childNodes[0].content;

        // Add the reservation to the list
        reservations.push(reservation);
    }

    return reservations;
}
