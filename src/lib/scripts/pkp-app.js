/*
 *   SPDX-FileCopyrightText: 2025 Grzegorz M
 *   SPDX-License-Identifier: LGPL-2.0-or-later
 */

function removeEmptyElements(arr) {
    return arr.filter(element => element !== undefined && element !== null && element !== '').filter(Boolean);
}

function main(content, node) {
    /**
     * Two main types of tickets:
     * - "wide": Single QR for multiple trains or one train.
     * - "narrow": Separate QR code for each train.
     * Names are made up.
     */

    const reservations = [];
    const contentLines = removeEmptyElements(content.text.split('\n'));

    const startRegex = /(?:odcinek|route)/g;
    const wideDataRegex = /\s*(ODJAZD|DEPARTURE)\s*(DATA|DATE)\s*(CZAS|TIME)\s*(POCIĄG|TRAIN)\s*(WAGON|CARRIAGE)/g;
    const narrowData1 = /^\s*(ODJAZD|DEPARTURE)\s*(DATA|DATE)/g;
    const arrivalDateAndMore = /\s*(PRZYJAZD|ARRIVAL)\s*(DATA|DATE)/g;
    const narrowData3 = /^\s*(CZAS|TIME)\s*(POCIĄG|TRAIN)\s*(WAGON|CARRIAGE)/g;
    const narrowData4 = /^\s*(MIEJSCA|SEATS)\s*$/g;
    const ticketNo = /^.*(TICKET NO|BILET NR)/g;

    const routeCutter = /^.*(odcinek:|route)/g;
    const classCutter = /(klasa|class)/g;

    const matches = [];
    let routeNo = 0;

    // Parse content lines
    contentLines.forEach((line, index) => {
        if (startRegex.test(line) || (routeNo === 0 && index === 7)) { // edge case for single route ticket
            routeNo++;
            matches[routeNo] = {
                routeNo,
                stations: contentLines[index].replace(routeCutter, '').trim(),
                reservationNumber: contentLines[6].replace(ticketNo, "").trim()
            };
        } else if (ticketNo.test(line) && routeNo > 0) { // its only valid for narrow tickets
            matches[routeNo].reservationNumber = line.replace(ticketNo, "").trim();
        } else if (wideDataRegex.test(line)) {
            matches[routeNo].wideData = removeEmptyElements(contentLines[index + 1].split(/\s{2,}/));
            matches[routeNo].wideData2 = removeEmptyElements(contentLines[index + 2].split(/\s{2,}/));
        } else if (narrowData1.test(line)) {
            matches[routeNo].stations = contentLines[index + 1];
            matches[routeNo].departureDate = removeEmptyElements(contentLines[index + 2].split(/\s{2,}/));
        } else if (arrivalDateAndMore.test(line)) {
            matches[routeNo].arrivalDateAndMore = removeEmptyElements(contentLines[index + 1].split(/\s{2,}/));
        } else if (narrowData3.test(line)) {
            matches[routeNo].timeAndTrain = removeEmptyElements(contentLines[index + 1].split(/\s{2,}/));
            matches[routeNo].timeAndTrain2 = removeEmptyElements(contentLines[index + 2].split(/\s{2,}/));
        } else if (narrowData4.test(line)) {
            matches[routeNo].seats = contentLines[index + 1].replace(/ +/g, ' ');
        }
    });

    console.log(`Number of routes: ${matches.length}`);

    matches.forEach((ticket, index) => {
        const reservation = JsonLd.newTrainReservation();

        reservation.reservationFor.departureStation.name = ticket.stations;
        reservation.reservationFor.arrivalStation.name = ticket.stations;

        if ('wideData' in ticket) {
            reservation.reservedTicket.ticketedSeat.seatSection = ticket.wideData[4];
            reservation.reservedTicket.ticketedSeat.seatingType = ticket.wideData2[1].replace(classCutter, '');
            reservation.reservationFor.trainName = ticket.wideData2[0];
            reservation.reservationFor.trainNumber = ticket.wideData[3];
            reservation.reservationFor.departureTime = JsonLd.toDateTime(
                ticket.wideData[0] + ticket.wideData[1], "hh:mmdd.MM.yyyy", "pl"
            );
            reservation.reservationFor.arrivalTime = JsonLd.toDateTime(
                ticket.arrivalDateAndMore[0] + ticket.arrivalDateAndMore[1], "hh:mmdd.MM.yyyy", "pl"
            );
            reservation.reservedTicket.ticketedSeat.seatNumber = ticket.arrivalDateAndMore.slice(2).join(" ");
            reservation.reservedTicket.ticketToken = 'azteccode:' + node.childNodes[1].childNodes[0].content;

            ExtractorEngine.extractPrice(contentLines[5], reservation);
        } else {
            // Narrow ticket
            reservation.reservedTicket.ticketedSeat.seatSection = ticket.timeAndTrain[2];
            reservation.reservedTicket.ticketedSeat.seatingType = ticket.timeAndTrain2[1].replace(classCutter, '');
            reservation.reservationFor.trainName = ticket.timeAndTrain2[0];
            reservation.reservationFor.trainNumber = ticket.timeAndTrain[1];
            reservation.reservationFor.departureTime = JsonLd.toDateTime(
                ticket.departureDate[0] + ticket.departureDate[1], "hh:mmdd.MM.yyyy", "pl"
            );
            reservation.reservationFor.arrivalTime = JsonLd.toDateTime(
                ticket.arrivalDateAndMore[0] + ticket.arrivalDateAndMore[1], "hh:mmdd.MM.yyyy", "pl"
            );
            reservation.reservedTicket.ticketedSeat.seatNumber = ticket.seats;
            reservation.reservedTicket.ticketToken = 'azteccode:' + node.childNodes[ticket.routeNo].childNodes[0].content;

            ExtractorEngine.extractPrice(contentLines[1].split(/\s{2,}/)[2], reservation);
        }

        reservation.reservationNumber = ticket.reservationNumber;
        reservation.reservationProvider = "PKP Intercity";

        reservations.push(reservation);
    });

    return reservations;
}
