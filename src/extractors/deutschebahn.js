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
    var seat = text.match(/Pl. ([\d ]+\d)/);
    if (seat)
        res.reservedTicket.ticketedSeat.seatNumber = seat[1];
}

function parseDeparture(res, line, year, compact) {
    var dep = line.match(/^(.+?) *([0-9]{2})\.([0-9]{2})\. +ab ([0-9]{2}:[0-9]{2})/);
    if (!dep)
        return false;

    res.reservationFor.departureStation.name = dep[1];
    res.reservationFor.departureTime = JsonLd.toDateTime(dep[2] + ' ' + dep[3] + ' ' + year + ' ' + dep[4], "dd MM yyyy hh:mm", "de");
    var idx = dep.index + dep[0].length;
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
    return true;
}

function parseArrival(res, line, year) {
    var arr = line.match(/^(.+?) *([0-9]{2})\.([0-9]{2})\. +an ([0-9]{2}:[0-9]{2})/);
    if (!arr)
        return false;
    res.reservationFor.arrivalStation.name = arr[1];
    res.reservationFor.arrivalTime = JsonLd.toDateTime(arr[2] + ' ' + arr[3] + ' ' + year + ' ' + arr[4], "dd MM yyyy hh:mm", "de");
    var idx = arr.index + arr[0].length;
    var platform = line.substr(idx).match(/^ {1,3}(.*?)(?=(  | IC|$))/);
    if (platform) {
        idx += platform.index + platform[0].length;
        res.reservationFor.arrivalPlatform = platform[1];
    }
    parseSeat(res, line.substr(idx));
    return true;
}

function parseLegs(text, year, compact) {
    var reservations = new Array();
    var lines = text.split('\n');
    var offset = lines[0].match(/^ */);
    for (var i = 0; compact && i < lines.length; ++i)
        lines[i] = lines[i].substr(offset[0].length);

    for (var i = 0; i < lines.length;) {
        // stop when reaching the footer or the next itinerary header
        if (isHeaderOrFooter(lines[i]))
            return reservations;

        var res = JsonLd.newTrainReservation();
        while (i < lines.length && !isHeaderOrFooter(lines[i])) {
            if (parseDeparture(res, lines[i++], year, compact))
                break;
        }
        while (i < lines.length && !isHeaderOrFooter(lines[i])) {
            if (parseArrival(res, lines[i], year)) {
                ++i;
                break;
            }

            // continuation of departure line
            var depStation = lines[i].match(/^(\S.*?)(?:  |\n|$)/)
            if (depStation)
                res.reservationFor.departureStation.name = res.reservationFor.departureStation.name +  " " + depStation[1];
            parseSeat(res, lines[i]);

            ++i;
        }
        // handle continuations of the arrival line
        while (i < lines.length && !isHeaderOrFooter(lines[i])) {
            if (lines[i].match(/^\S.+? *[0-9]{2}\.[0-9]{2}\. +ab [0-9]{2}:[0-9]{2}/)) // next departure line
                break;

            // continuation of arrival line
            var arrStation = lines[i].match(/^(\S.*?)(?:  |\n|$)/)
            if (arrStation)
                res.reservationFor.arrivalStation.name = res.reservationFor.arrivalStation.name +  " " + arrStation[1];
            parseSeat(res, lines[i]);

            ++i;
        }

        if (res.reservationFor.arrivalStation != undefined) {
            reservations.push(res);
        } else {
            ++i;
        }
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
        if (uic918ticket && header[1] === "Hin") {
            reservations[0].reservationFor.departureStation.identifier = uic918ticket.outboundDepartureStationId;
            reservations[reservations.length - 1].reservationFor.arrivalStation.identifier = uic918ticket.outboundArrivalStationId;
            returnResIndex = reservations.length;
        } else {
            // propagate station ids from outward to return journey
            for (var i = returnResIndex; i < reservations.length; ++i) {
                for (var j = 0; j < returnResIndex; ++j) {
                    if (reservations[i].reservationFor.departureStation.name === reservations[j].reservationFor.arrivalStation.name)
                        reservations[i].reservationFor.departureStation.identifier = reservations[j].reservationFor.arrivalStation.identifier;
                    if (reservations[i].reservationFor.arrivalStation.name === reservations[j].reservationFor.departureStation.name)
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
    var bookingRef = text.match(/(?:Auftragsnummer|Auftrag \(NVS\)):\s*([A-Z0-9]{6,9})\n/);
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
        if (images[i].width === images[i].height) {
            barcode = Barcode.decodeAztecBinary(images[i]);
            uic918ticket = Barcode.decodeUic9183(barcode);
            break;
        }
    }

    var reservations = parseTicket(pdf.text, uic918ticket);
    for (var i = 0; i < reservations.length && barcode; ++i) {
        reservations[i].reservedTicket.ticketToken = "aztecbin:" + Barcode.toBase64(barcode);
        reservations[i].reservedTicket.ticketedSeat.seatingType = uic918ticket.seatingType;
        reservations[i].underName = JsonLd.toJson(uic918ticket.person);
    }
    return reservations;
}
