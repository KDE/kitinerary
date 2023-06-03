/*
   SPDX-FileCopyrightText: 2017 Volker Krause <vkrause@kde.org>
   SPDX-FileCopyrightText: 2018 Daniel Vrátil <dvratil@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

function isHeaderOrFooter(line) {
    return line.search(/(Jízdní řád( a rezervace)?|Jízdenku lze použít po stejné trase|Jízdní doklad zakoupený u obchodníka)/) >= 0;
}

function createSeat(res)
{
    if (!res.reservedTicket)
        res.reservedTicket = JsonLd.newObject("Ticket");
    if (!res.reservedTicket.ticketedSeat)
        res.reservedTicket.ticketedSeat = JsonLd.newObject("Seat");
}

function parseSeat(res, text) {
    var coach = text.match(/(\s+)(\d+)/);
    var idx = 0;
    if (coach) {
        createSeat(res);
        res.reservedTicket.ticketedSeat.seatSection = coach[2];
        idx = coach.index + coach[1].length + coach[2].length;
    }
    var seat = text.substr(idx).match(/\s+([\d,\- ]+)/)
    if (seat && seat[1].trim()) {
        seatNumber = seat[1].trim();
        if (seatNumber) {
            createSeat(res);
            res.reservedTicket.ticketedSeat.seatNumber = seatNumber;
        }
    }
}

// There's no validity year anywhere in the ticket, so we take the purchase date and
// if the trip month and day are after the purchase month and day we assume the
// the ticket will become valid the same year it was purchased, otherwise we assume
// the ticket is for next year.
// This fails when you buy the ticket more than a year ahead of the trip, but I doubt
// you can even do that with Czech Railways...
function detectYear(tripDate, purchaseDate)
{
    var tripDay = parseInt(tripDate[1]);
    var tripMonth = parseInt(tripDate[2]);
    var purchaseDay = parseInt(purchaseDate[2]);
    var purchaseMonth = parseInt(purchaseDate[3]);
    var purchaseYear = parseInt(purchaseDate[4]);

    if ((purchaseMonth < tripMonth) ||
        (purchaseMonth === tripMonth) && (purchaseDay <= tripDay)) {
        return purchaseYear;
    } else {
        return purchaseYear + 1;
    }
}

function parseDeparture(res, line, purchaseDate) {
    res.reservationFor.departureStation = JsonLd.newObject("TrainStation");
    var station = line.match(/^(.+?)  /);
    if (!station)
        return;
    var idx = station.index + station[0].length;
    res.reservationFor.departureStation.name = station[1];
    var dt = line.substr(idx).match(/([0-9]{2})\.([0-9]{2})\. ([0-9]{2}:[0-9]{2})/);
    if (dt) {
        idx += dt.index + dt[0].length;
        res.reservationFor.departureTime = JsonLd.toDateTime(dt[1] + ' ' + dt[2] + ' ' + detectYear(dt, purchaseDate) + ' ' + dt[3], "dd MM yyyy hh:mm", "cs");
    }
    var trainId = line.substr(idx).match(/([a-zA-Z]+ [0-9a-zA-Z]+)/);
    if (trainId) {
        idx += trainId.index + trainId[0].length
        res.reservationFor.trainNumber = trainId[1];
    }
    parseSeat(res, line.substr(idx));
}

function parseArrival(res, line, purchaseDate) {
    res.reservationFor.arrivalStation = JsonLd.newObject("TrainStation");
    var station = line.match(/^(.+?)  /);
    if (!station)
        return;
    var idx = station.index + station[0].length;
    res.reservationFor.arrivalStation.name = station[1];
    var dt = line.substr(idx).match(/([0-9]{2})\.([0-9]{2})\. ([0-9]{2}:[0-9]{2})/);
    if (dt) {
        idx += dt.index + dt[0].length;
        res.reservationFor.arrivalTime = JsonLd.toDateTime(dt[1] + ' ' + dt[2] + ' ' + detectYear(dt, purchaseDate) + ' ' + dt[3], "dd MM yyyy hh:mm", "cs");
    }
}

function findNextStationLineIndex(lines, start) {
    var pos = start;
    while (pos < lines.length && (lines[pos].startsWith(" ") || lines[pos].startsWith("Ref:"))) {
        pos += 1;
    }
    return pos;
}

function parseLegs(text, purchaseDate) {
    var reservations = new Array();
    var lines = text.split('\n');
    var depIdx = 1, arrIdx = 2;
    while (depIdx < lines.length) {
        // stop when reaching the footer or the next itinerary header
        if (isHeaderOrFooter(lines[depIdx]))
            return reservations;

        var res = JsonLd.newObject("TrainReservation");
        res.reservationFor = JsonLd.newObject("TrainTrip");

        arrIdx = findNextStationLineIndex(lines, depIdx + 1)
        parseDeparture(res, lines[depIdx], purchaseDate);
        parseArrival(res, lines[arrIdx], purchaseDate);

        // Find the next leg
        depIdx = findNextStationLineIndex(lines, arrIdx + 1);

        reservations.push(res);
    }

    return reservations;
}

function main(text) {
    var reservations = new Array();
    var pos = 0;

    var purchaseDate = text.match(/([d|D]atum platby|UZP): ([0-9]{1,2})\.([0-9]{1,2})\.([0-9]{4})/)

    while (true) {
         // find itinerary headers
        var header = text.substr(pos).match(/Timetable( and Reservations)?/);
        if (!header)
            break;
        var idx = header.index + header[0].length;
        var timetableHeader = text.substr(pos + idx).match(/(Místo \/ Seat \/ Sitzplatz)/)
        idx = idx + timetableHeader.index + timetableHeader[0].length;
        reservations = reservations.concat(parseLegs(text.substr(pos + idx), purchaseDate));
        if (idx == 0)
            break;
        pos += idx + 1;
    }

    var bookingRef = text.match(/Kód transakce:\s*([A-Z0-9]{6})\n/);
    for (var i = 0; bookingRef && i < reservations.length; ++i)
        reservations[i].reservationNumber = bookingRef[1];
    return reservations;
}

// ### this is based on an international ticket, do domestic ones look the same?
function parsePdfTicket(content, node, triggerNode) {
    const page = content.pages[triggerNode.location];
    const text = page.textInRect(0.0, 0.0, 0.675, 1.0);
    let idx = 0;
    let reservations = [];
    while (true) {
        let leg = text.substr(idx).match(/\n(.*?)  +(\d\d\.\d\d\.) +(\d\d:\d\d) +(.*?)(?:  +(\d+) +(\d+))?\n(.*?)  +(\d\d.\d\d.) +(\d\d:\d\d)/);
        if (!leg) {
            break;
        }
        let res = JsonLd.newTrainReservation();
        res.reservationFor.departureStation.name = leg[1];
        res.reservationFor.departureTime = JsonLd.toDateTime(leg[2] + leg[3], 'dd.MM.hh:mm', 'cz');
        res.reservationFor.arrivalStation.name = leg[7];
        res.reservationFor.arrivalTime = JsonLd.toDateTime(leg[8] + leg[9], 'dd.MM.hh:mm', 'cz');
        res.reservationFor.trainNumber = leg[4];
        if (leg[5] && leg[6]) {
            res.reservedTicket.ticketedSeat.seatSection = leg[5];
            res.reservedTicket.ticketedSeat.seatNumber = leg[6];
        }

        if (triggerNode.result[0]['@type'] == 'TrainReservation') {
            res = JsonLd.apply(triggerNode.result[0], res);
            res.reservationNumber = triggerNode.content.block('1154UT').findSubBlock('KK').content;
        }
        res.reservedTicket = JsonLd.apply(triggerNode.result[0].reservedTicket, res.reservedTicket);

        reservations.push(res);
        idx += leg.index + leg[0].length;
    }

    return reservations;
}

function parseUicTicket(uic, node)
{
    if (node.result[0]['@type'] == 'TrainReservation') {
        let res = node.result[0];
        res.reservationNumber = uic.block('1154UT').findSubBlock('KK').content;
        res.reservedTicket.ticketNumber = uic.pnr;
        return res;
    } else {
        let ticket = node.result[0];
        ticket.ticketNumber = uic.pnr;
        return ticket;
    }
}

function parseEvent(event) {
    let reservations = [];
    let idx = 0;
    while (true) {
        const leg = event.description.substr(idx).match(/(.*)\n(.*) (\d\d):(\d\d)\n(.*) (\d\d):(\d\d)\n.* (\d+),.* (\d+)/);
        if (!leg)
            break;
        idx += leg.index + leg[0].length;

        let res = JsonLd.newTrainReservation();
        res.reservationFor.trainNumber = leg[1];
        res.reservationFor.departureStation.name = leg[2];
        res.reservationFor.departureTime = new Date(event.dtStart);
        res.reservationFor.departureTime.setHours(leg[3], leg[4]);
        res.reservationFor.arrivalStation.name = leg[5];
        res.reservationFor.arrivalTime = new Date(event.dtStart);
        res.reservationFor.arrivalTime.setHours(leg[6], leg[7]);
        res.reservedTicket.ticketedSeat.seatSection = leg[8];
        res.reservedTicket.ticketedSeat.seatNumber = leg[9];
        reservations.push(res);
    }
    return reservations;
}
