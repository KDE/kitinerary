/*
 *   SPDX-FileCopyrightText: 2025 Grzegorz M
 *   SPDX-License-Identifier: LGPL-2.0-or-later
 */

function removeEmptyElements(arr) {
    return arr.filter(element => element !== undefined && element !== null && element !== '').filter(Boolean);
}

const lineMatchers = {
    routeStart: /odcinek|route/,
    wideHeader: /\s*(ODJAZD|DEPARTURE)\s*(DATA|DATE)\s*(CZAS|TIME)\s*(POCIĄG|TRAIN)\s*(WAGON|CARRIAGE)/,
    narrowDepartureHeader: /^\s*(ODJAZD|DEPARTURE)\s*(DATA|DATE)/,
    arrivalHeader: /\s*(PRZYJAZD|ARRIVAL)\s*(DATA|DATE)/,
    narrowTrainHeader: /^\s*(CZAS|TIME)\s*(POCIĄG|TRAIN)\s*(WAGON|CARRIAGE)/,
    narrowSeatsHeader: /^\s*(MIEJSCA|SEATS)\s*$/,
    ticketNumber: /^.*(TICKET NO|BILET NR)/,
    routePrefix: /^.*(odcinek:|route)/,
    ticketClass: /(klasa|class)/g
};

const ticketLayoutHandlers = {
    wide: applyWideLayout,
    narrow: applyNarrowLayout
};

function columns(line) {
    return removeEmptyElements((line || '').split(/\s{2,}/));
}

function ticketNumberFrom(line) {
    return (line || '').replace(lineMatchers.ticketNumber, '').trim();
}

function routeNameFrom(line) {
    return (line || '').replace(lineMatchers.routePrefix, '').trim();
}

function isFirstRouteFallback(routeCount, lineIndex) {
    return routeCount === 0 && lineIndex === 7;
}

function parseTickets(contentLines) {
    const tickets = [];

    contentLines.forEach((line, index) => {
        let ticket = tickets[tickets.length - 1];

        if (lineMatchers.routeStart.test(line) || isFirstRouteFallback(tickets.length, index)) {
            ticket = {
                routeNo: tickets.length + 1,
                stations: routeNameFrom(line),
                reservationNumber: ticketNumberFrom(contentLines[6])
            };
            tickets.push(ticket);
            return;
        }

        if (!ticket) {
            return;
        }

        if (lineMatchers.ticketNumber.test(line)) { // only valid for narrow tickets
            ticket.reservationNumber = ticketNumberFrom(line);
        } else if (lineMatchers.wideHeader.test(line)) {
            ticket.layout = 'wide';
            ticket.wideData = columns(contentLines[index + 1]);
            ticket.wideData2 = columns(contentLines[index + 2]);
        } else if (lineMatchers.narrowDepartureHeader.test(line)) {
            ticket.layout = 'narrow';
            ticket.stations = contentLines[index + 1];
            ticket.departureDate = columns(contentLines[index + 2]);
        } else if (lineMatchers.arrivalHeader.test(line)) {
            ticket.arrivalDateAndMore = columns(contentLines[index + 1]);
        } else if (lineMatchers.narrowTrainHeader.test(line)) {
            ticket.layout = 'narrow';
            ticket.timeAndTrain = columns(contentLines[index + 1]);
            ticket.timeAndTrain2 = columns(contentLines[index + 2]);
        } else if (lineMatchers.narrowSeatsHeader.test(line)) {
            ticket.layout = 'narrow';
            ticket.seats = (contentLines[index + 1] || '').replace(/ +/g, ' ');
        }
    });

    return tickets;
}

function detectLayout(ticket) {
    if (ticket.layout) {
        return ticket.layout;
    }
    if ('wideData' in ticket) {
        return 'wide';
    }
    return 'narrow';
}

function ticketToken(node, childIndex) {
    return 'azteccode:' + node.childNodes[childIndex].childNodes[0].content;
}

function setDateTime(target, propertyName, parts) {
    target[propertyName] = JsonLd.toDateTime(parts[0] + parts[1], "hh:mmdd.MM.yyyy", "pl");
}

function applyCommonReservationData(reservation, ticket) {
    reservation.reservationFor.departureStation.name = ticket.stations;
    reservation.reservationFor.arrivalStation.name = ticket.stations;
    reservation.reservationNumber = ticket.reservationNumber;
    reservation.reservationProvider = "PKP Intercity";
}

function applyWideLayout(reservation, ticket, context) {
    reservation.reservedTicket.ticketedSeat.seatSection = ticket.wideData[4];
    reservation.reservedTicket.ticketedSeat.seatingType = ticket.wideData2[1].replace(lineMatchers.ticketClass, '');
    reservation.reservationFor.trainName = ticket.wideData2[0];
    reservation.reservationFor.trainNumber = ticket.wideData[3];
    setDateTime(reservation.reservationFor, 'departureTime', ticket.wideData);
    setDateTime(reservation.reservationFor, 'arrivalTime', ticket.arrivalDateAndMore);
    reservation.reservedTicket.ticketedSeat.seatNumber = ticket.arrivalDateAndMore.slice(2).join(" ");
    reservation.reservedTicket.ticketToken = ticketToken(context.node, 1);

    ExtractorEngine.extractPrice(context.contentLines[5], reservation);
}

function applyNarrowLayout(reservation, ticket, context) {
    reservation.reservedTicket.ticketedSeat.seatSection = ticket.timeAndTrain[2];
    reservation.reservedTicket.ticketedSeat.seatingType = ticket.timeAndTrain2[1].replace(lineMatchers.ticketClass, '');
    reservation.reservationFor.trainName = ticket.timeAndTrain2[0];
    reservation.reservationFor.trainNumber = ticket.timeAndTrain[1];
    setDateTime(reservation.reservationFor, 'departureTime', ticket.departureDate);
    setDateTime(reservation.reservationFor, 'arrivalTime', ticket.arrivalDateAndMore);
    reservation.reservedTicket.ticketedSeat.seatNumber = ticket.seats;
    reservation.reservedTicket.ticketToken = ticketToken(context.node, ticket.routeNo);

    ExtractorEngine.extractPrice(columns(context.contentLines[1])[2], reservation);
}

function reservationFromTicket(ticket, context) {
    const reservation = JsonLd.newTrainReservation();
    const layout = detectLayout(ticket);

    applyCommonReservationData(reservation, ticket);
    ticketLayoutHandlers[layout](reservation, ticket, context);

    return reservation;
}

function main(content, node) {
    /**
     * Known ticket layouts:
     * - "wide": Single QR for multiple trains or one train.
     * - "narrow": Separate QR code for each train.
     * Names are made up.
     *
     * To add another incompatible layout, keep the common route parsing here and add:
     * - a parser branch in parseTickets() that sets ticket.layout
     * - a layout function in ticketLayoutHandlers
     */
    const contentLines = removeEmptyElements(content.text.split('\n'));
    const tickets = parseTickets(contentLines);

    console.log(`Number of routes: ${tickets.length}`);

    return tickets.map(ticket => reservationFromTicket(ticket, { contentLines, node }));
}

function fixStationCode(station) {
    // UIC codes in FCB barcodes in Germany are wildly unreliable, there seem to be different
    // code tables in use by different operators, so we unfortunately have to ignore
    // those entirely
    if (!station.identifier || !station.identifier.startsWith("uic:80"))
        return;
    station.identifier = undefined;
    station.address = {
        "@type": "PostalAddress",
        addressCountry: "DE"
    };
}

function fixFCB(code, node)
{
    if (code.block("U_FLEX")) {
        let result = node.result;
        for (res of result) {
            fixStationCode(res.reservationFor.departureStation);
            fixStationCode(res.reservationFor.arrivalStation);
        }
        return result;
    }
}
