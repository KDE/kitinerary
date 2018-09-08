/*
   Copyright (c) 2017 Volker Krause <vkrause@kde.org>

   This library is free software; you can redistribute it and/or modify it
   under the terms of the GNU Library General Public License as published by
   the Free Software Foundation; either version 2 of the License, or (at your
   option) any later version.

   This library is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
   FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
   License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to the
   Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
   02110-1301, USA.
*/

function isHeaderOrFooter(line) {
    return line.search(/(Ihre Reiseverbindung|Wichtige Nutzungshinweise|Hinweise:|Seite \d \/ \d)/) >= 0;
}

function parseSeat(res, text) {
    var coach = text.match(/Wg. (\d+)/);
    if (coach)
        res.reservedTicket.ticketedSeat.seatSection = coach[1];
    var seat = text.match(/Pl. (\d+)/);
    if (seat)
        res.reservedTicket.ticketedSeat.seatNumber = seat[1];
}

function parseDeparture(res, line, year, compact) {
    res.reservationFor.departureStation = JsonLd.newObject("TrainStation");
    var station = line.match(/^(.+?)  /);
    if (!station)
        return;
    var idx = station.index + station[0].length;
    res.reservationFor.departureStation.name = station[1];
    var dt = line.substr(idx).match(/([0-9]{2})\.([0-9]{2})\. +ab ([0-9]{2}:[0-9]{2})/);
    if (dt) {
        idx += dt.index + dt[0].length;
        res.reservationFor.departureTime = JsonLd.toDateTime(dt[1] + ' ' + dt[2] + ' ' + year + ' ' + dt[3], "dd MM yyyy hh:mm", "de");
    }
    var platform = line.substr(idx).match(/^ {1,3}(.*?)(?=(  | IC|$))/);
    if (platform) {
        idx += platform.index + platform[0].length;
        res.reservationFor.departurePlatform = platform[1];
    }
    var trainId = line.substr(idx).match(compact ? / +([^,]*?)(?=(,|$))/ : / +(.*?)(?=(  |$))/);
    if (trainId) {
        idx += trainId.index + trainId[0].length
        res.reservationFor.trainNumber = trainId[1];
    }
    parseSeat(res, line.substr(idx));
}

function parseArrival(res, line, year) {
    res.reservationFor.arrivalStation = JsonLd.newObject("TrainStation");
    var station = line.match(/^(.+?)  /);
    if (!station)
        return;
    var idx = station.index + station[0].length;
    res.reservationFor.arrivalStation.name = station[1];
    var dt = line.substr(idx).match(/([0-9]{2})\.([0-9]{2})\. +an ([0-9]{2}:[0-9]{2})/);
    if (dt) {
        idx += dt.index + dt[0].length;
        res.reservationFor.arrivalTime = JsonLd.toDateTime(dt[1] + ' ' + dt[2] + ' ' + year + ' ' + dt[3], "dd MM yyyy hh:mm", "de");
    }
    var platform = line.substr(idx).match(/^ {1,3}(.*?)(?=(  | IC|$))/);
    if (platform) {
        idx += platform.index + platform[0].length;
        res.reservationFor.arrivalPlatform = platform[1];
    }
    parseSeat(res, line.substr(idx));
}

function parseLegs(text, year, compact) {
    var reservations = new Array();
    var lines = text.split('\n');
    var offset = lines[0].match(/^ */);
    for (var i = 0; compact && i < lines.length; ++i)
        lines[i] = lines[i].substr(offset[0].length);

    var depIdx = 0, arrIdx = 1;
    while (depIdx < lines.length) {
        var res = JsonLd.newObject("TrainReservation");
        res.reservationFor = JsonLd.newObject("TrainTrip");
        res.reservedTicket = JsonLd.newObject("Ticket");
        res.reservedTicket.ticketedSeat = JsonLd.newObject("Seat");

        // stop when reaching the footer or the next itinerary header
        if (isHeaderOrFooter(lines[depIdx]))
            return reservations;

        arrIdx = depIdx + 1;
        while (arrIdx < lines.length && lines[arrIdx].startsWith(' ')) // line continuations for departure, we still need to handle that correctly
            ++arrIdx;
        parseDeparture(res, lines[depIdx], year, compact);
        parseArrival(res, lines[arrIdx], year);
        depIdx = arrIdx + 1;
        while (depIdx < lines.length && lines[depIdx].startsWith(' ') && !isHeaderOrFooter(lines[depIdx])) // line continuations for arrival, dito
            ++depIdx;
        reservations.push(res);
    }

    return reservations;
}

function parseText(text) { // used by unit tests
    return parseTicket(text, null);
}

function parseTicket(text, uic918ticket) {
    var reservations = new Array();
    var pos = 0;
    var returnResIndex = 0;
    while (true) {
        // find itinerary headers
        var header = text.substr(pos).match(/Ihre Reiseverbindung[\S ]+(Hin|Rück)fahrt am [0-9]{2}.[0-9]{2}.([0-9]{4}).*\n/);
        if (!header)
            break;
        var idx = header.index + header[0].length;
        var year = header[2];

        // determine ticket type
        var domesticHeader = text.substr(pos + idx).match(/  Reservierung\n/);
        var intlHeader = text.substr(pos + idx).match(/(Produkte\/Reservierung|Fahrt\/Reservierung).*\n/);
        if (domesticHeader) {
            idx += domesticHeader.index + domesticHeader[0].length;
            reservations = reservations.concat(parseLegs(text.substr(pos + idx), year, false));
        } else if (intlHeader) {
            idx += intlHeader.index + intlHeader[0].length;
            reservations = reservations.concat(parseLegs(text.substr(pos + idx), year, true));
        } else {
            break;
        }

        // for outward journeys we have station ids from the UIC 918-3 code
        if (uic918ticket && header[1] == "Hin") {
            reservations[0].reservationFor.departureStation.identifier = uic918ticket.outboundDepartureStationId;
            reservations[reservations.length - 1].reservationFor.arrivalStation.identifier = uic918ticket.outboundArrivalStationId;
            returnResIndex = reservations.length;
        } else {
            // propagate station ids from outward to return journey
            for (var i = returnResIndex; i < reservations.length; ++i) {
                for (var j = 0; j < returnResIndex; ++j) {
                    if (reservations[i].reservationFor.departureStation.name == reservations[j].reservationFor.arrivalStation.name)
                        reservations[i].reservationFor.departureStation.identifier = reservations[j].reservationFor.arrivalStation.identifier;
                    if (reservations[i].reservationFor.arrivalStation.name == reservations[j].reservationFor.departureStation.name)
                        reservations[i].reservationFor.arrivalStation.identifier = reservations[j].reservationFor.departureStation.identifier;
                }
            }
        }

        if (idx == 0)
            break;
        pos += idx;
    }

    // international tickets have the booking reference somewhere on the side, so we don't really know
    // where it is relative to the itinerary
    var bookingRef = text.match(/Auftragsnummer:\s*([A-Z0-9]{6})\n/);
    for (var i = 0; i < reservations.length; ++i)
        reservations[i].reservationNumber = bookingRef[1];
    return reservations;
}

function parsePdf(pdf) {
    // try to find the UIC918.3 barcode
    var images = pdf.pages[0].imagesInRect(0.6, 0, 1, 1);
    var barcode = null;
    var uic918ticket = null;
    for (var i = 0; i < images.length && !barcode; ++i) {
        if (images[i].width == images[i].height) {
            barcode = Barcode.decodeAztecBinary(images[i]);
            uic918ticket = Barcode.decodeUic9183(barcode);
            break;
        }
    }

    console.log(barcode, uic918ticket, !barcode);
    var reservations = parseTicket(pdf.text, uic918ticket);
    for (var i = 0; i < reservations.length && barcode; ++i) {
        reservations[i].reservedTicket.ticketToken = "aztecbin:" + Barcode.toBase64(barcode);
        reservations[i].underName = JsonLd.toJson(uic918ticket.person);
    }
    return reservations;
}
