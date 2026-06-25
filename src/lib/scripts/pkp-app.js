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
    newDateHeader: /(Data odjazdu|Departure)\s*\/\s*(przyjazdu|arrival date):/,
    newTimeHeader: /(Godzina odjazdu|Departure)\s*\/\s*(przyjazdu|arrival time):/,
    newTrainHeader: /(Pociąg|Train):/,
    newCarriageHeader: /(Wagon|Carriage):/,
    newSeatsHeader: /(Miejsca|Seats):/,
    newRouteStart: /^\s*\d+\.\s*(odcinek|route):/,
    newRoutePrefix: /^.*(?:odcinek:|route:)/,
    ticketNumber: /^.*(TICKET NO|BILET NR)/,
    routePrefix: /^.*(odcinek:|route)/,
    ticketClass: /(klasa|class)/g
};

const ticketLayoutHandlers = {
    wide: applyWideLayout,
    narrow: applyNarrowLayout,
    app2026: applyApp2026Layout
};

function columns(line) {
    return removeEmptyElements((line || '').split(/\s{2,}/));
}

function cut(line, regexp) {
    return (line || '').replace(regexp, '').trim();
}

function parseOldTickets(contentLines) {
    const tickets = [];

    contentLines.forEach((line, index) => {
        let ticket = tickets[tickets.length - 1];

        const singleRouteTicket= (tickets.length === 0 && index === 7)
        if (lineMatchers.routeStart.test(line) || singleRouteTicket) {
            ticket = {
                routeNo: tickets.length + 1,
                stations: cut(line, lineMatchers.routePrefix),
                reservationNumber: cut(contentLines[6], lineMatchers.ticketNumber)
            };
            tickets.push(ticket);
            return;
        }

        if (!ticket) {
            return;
        }

        if (lineMatchers.ticketNumber.test(line)) { // only valid for narrow tickets
            ticket.reservationNumber = cut(line, lineMatchers.ticketNumber);
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

function hasApp2026Layout(contentLines) {
    return [lineMatchers.newDateHeader, lineMatchers.newTimeHeader, lineMatchers.newTrainHeader]
        .every(regexp => contentLines.some(line => regexp.test(line)));
}

function parseApp2026Fields(line, ticket) {
    if (lineMatchers.newDateHeader.test(line)) {
        [ticket.departureDate, ticket.arrivalDate] = cut(line, /^.*:/).split(/\s+/);
    } else if (lineMatchers.newTimeHeader.test(line)) {
        [ticket.departureTime, ticket.arrivalTime] = cut(line, /^.*?:/).split(/\s+/);
    } else if (lineMatchers.newTrainHeader.test(line)) {
        const train = cut(line, /^.*?:/).match(/^(\S+)\s*(.*)$/);
        ticket.trainNumber = train ? train[1] : undefined;
        ticket.trainName = train ? train[2].trim() : undefined;
    } else if (lineMatchers.newCarriageHeader.test(line)) {
        ticket.seatSection = (line.match(/(?:Wagon|Carriage):\s*([^,]+)/) || [])[1];
        ticket.seatingType = (line.match(/(?:klasa|class)\s*(\S+)/) || [])[1];
    } else if (lineMatchers.newSeatsHeader.test(line)) {
        ticket.seatNumber = (line.match(/(?:Miejsca|Seats):\s*(.*?)(?:\s{2,}|$)/) || [])[1];
    }
}

function parseApp2026Ticket(contentLines) {
    const hasRouteSections = contentLines.some(line => lineMatchers.newRouteStart.test(line));
    const tickets = hasRouteSections ? [] : [{ layout: 'app2026', routeNo: 1 }];
    let reservationNumber;
    let ticket = tickets[0];

    contentLines.forEach((line, index) => {
        if (lineMatchers.ticketNumber.test(line)) {
            reservationNumber = cut(line, lineMatchers.ticketNumber).replace(/KOD.*$/, '').trim();
            tickets.forEach(t => { t.reservationNumber = reservationNumber; });
            if (ticket && !hasRouteSections) {
                ticket.stations = (contentLines[index + 1] || '').trim();
            }
        } else if (lineMatchers.newRouteStart.test(line)) {
            ticket = {
                layout: 'app2026',
                routeNo: tickets.length + 1,
                reservationNumber,
                stations: cut(line, lineMatchers.newRoutePrefix)
            };
            tickets.push(ticket);
        } else {
            if (ticket) {
                parseApp2026Fields(line, ticket);
            }
        }
    });

    tickets.forEach(t => { t.reservationNumber = t.reservationNumber || reservationNumber; });
    return tickets;
}

function ticketToken(node, childIndex) {
    return 'azteccode:' + node.childNodes[childIndex].childNodes[0].content;
}

function setDateTime(target, propertyName, parts) {
    target[propertyName] = JsonLd.toDateTime(parts[0] + parts[1], "hh:mmdd.MM.yyyy", "pl");
}

function setTrain(reservation, trainNumber, trainName) {
    reservation.reservationFor.trainNumber = trainNumber;
    reservation.reservationFor.trainName = trainName;
}

function setSeat(reservation, seatSection, seatingType, seatNumber) {
    const seat = reservation.reservedTicket.ticketedSeat;
    seat.seatSection = seatSection;
    seat.seatingType = seatingType;
    seat.seatNumber = seatNumber;
}

function applyWideLayout(reservation, ticket, context) {
    setSeat(reservation, ticket.wideData[4], ticket.wideData2[1].replace(lineMatchers.ticketClass, ''), ticket.arrivalDateAndMore.slice(2).join(" "));
    setTrain(reservation, ticket.wideData[3], ticket.wideData2[0]);
    setDateTime(reservation.reservationFor, 'departureTime', ticket.wideData);
    setDateTime(reservation.reservationFor, 'arrivalTime', ticket.arrivalDateAndMore);
    reservation.reservedTicket.ticketToken = ticketToken(context.node, 1);

    ExtractorEngine.extractPrice(context.contentLines[5], reservation);
}

function applyNarrowLayout(reservation, ticket, context) {
    setSeat(reservation, ticket.timeAndTrain[2], ticket.timeAndTrain2[1].replace(lineMatchers.ticketClass, ''), ticket.seats);
    setTrain(reservation, ticket.timeAndTrain[1], ticket.timeAndTrain2[0]);
    setDateTime(reservation.reservationFor, 'departureTime', ticket.departureDate);
    setDateTime(reservation.reservationFor, 'arrivalTime', ticket.arrivalDateAndMore);
    reservation.reservedTicket.ticketToken = ticketToken(context.node, ticket.routeNo);

    ExtractorEngine.extractPrice(columns(context.contentLines[1])[2], reservation);
}

function applyApp2026Layout(reservation, ticket, context) {
    setSeat(reservation, ticket.seatSection, ticket.seatingType, ticket.seatNumber);
    setTrain(reservation, ticket.trainNumber, ticket.trainName);
    setDateTime(reservation.reservationFor, 'departureTime', [ticket.departureTime, ticket.departureDate]);
    setDateTime(reservation.reservationFor, 'arrivalTime', [ticket.arrivalTime, ticket.arrivalDate]);
    reservation.reservedTicket.ticketToken = ticketToken(context.node, 1);

    ExtractorEngine.extractPrice(context.contentLines[1], reservation);
}

function reservationFromTicket(ticket, context) {
    const reservation = JsonLd.newTrainReservation();

    reservation.reservationFor.departureStation.name = ticket.stations;
    reservation.reservationFor.arrivalStation.name = ticket.stations;
    reservation.reservationNumber = ticket.reservationNumber;
    reservation.reservationProvider = "PKP Intercity";

    ticketLayoutHandlers[ticket.layout || ('wideData' in ticket ? 'wide' : 'narrow')](reservation, ticket, context);

    return reservation;
}

function main(content, node) {
    /**
     * Known ticket layouts:
     * - "wide": Single QR for multiple trains or one train.
     * - "narrow": Separate QR code for each train.
     * - "app2026": Randomly pkp changed ticket layout in early 2026 (previous wide)
     * Names are made up.
     * ps. app2026 narrow ticket type was not found yet
     *
     */
    const contentLines = removeEmptyElements(content.text.split('\n'));
    const tickets = hasApp2026Layout(contentLines) ? parseApp2026Ticket(contentLines) : parseOldTickets(contentLines);

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
