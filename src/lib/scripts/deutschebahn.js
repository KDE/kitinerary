/*
   SPDX-FileCopyrightText: 2017 Volker Krause <vkrause@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
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

function convertToBus(res) {
    res['@type'] = 'BusReservation';
    res.reservationFor['@type'] = 'BusTrip';
    res.reservationFor.departureBusStop = res.reservationFor.departureStation;
    res.reservationFor.departureBusStop['@type'] = 'BusStop';
    res.reservationFor.arrivalBusStop = res.reservationFor.arrivalStation;
    res.reservationFor.arrivalBusStop['@type'] = 'BusStop';
    res.reservationFor.busName = res.reservationFor.trainName;
    res.reservationFor.busNumber = res.reservationFor.trainNumber;
    res.reservedTicket.ticketedSeat = undefined;
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
    for (var i = 0; i < reservations.length; ++i) {
        reservations[i].reservationNumber = bookingRef[1];

        if (reservations[i].reservationFor.trainNumber && reservations[i].reservationFor.trainNumber.startsWith("Bus")) {
            convertToBus(reservations[i]);
        }
    }
    return reservations;
}

function parseReservation(pdf) {
    var text = pdf.text;
    var reservations = Array();
    var idx = 0;

    while (true) {
        var dep = text.substr(idx).match(/  (\S.*\S) +(\d\d.\d\d.\d\d) +ab (\d\d:\d\d)  +(.*?)  +([A-Z].*\S)  +(.*)\n/);
        if (!dep)
            return reservations;
        idx += dep.index + dep[0].length;
        var arr = text.substr(idx).match(/  (\S.*\S) +an (\d\d:\d\d)  +(.*?)  +(.*)\n/);
        if (!arr)
            return reservations;

        var res = JsonLd.newTrainReservation();
        res.reservationFor.departureStation.name = dep[1];
        res.reservationFor.departureTime = JsonLd.toDateTime(dep[2] + dep[3], "dd.MM.yyhh:mm", "de");
        res.reservationFor.trainNumber = dep[5];
        res.reservationFor.departurePlatform = dep[4];
        res.reservationFor.arrivalStation.name = arr[1];
        res.reservationFor.arrivalTime = JsonLd.toDateTime(dep[2] + arr[2], "dd.MM.yyhh:mm", "de");
        res.reservationFor.arrivalPlatform = arr[3];
        reservations.push(res);

        seatText = dep[6] + " " + arr[4];
        var seat = seatText.match(/(\d)\. Klasse, Wg\. (\d+), Pl. (.*?),/);
        if (seat) {
            res.reservedTicket.ticketedSeat.seatingType = seat[1];
            res.reservedTicket.ticketedSeat.seatSection = seat[2];
            res.reservedTicket.ticketedSeat.seatNumber = seat[3];
        }

        idx += arr.index + arr[0].length;
    }

    return reservations;
}

function parsePdf(pdf, node, triggerNode) {
    var page = pdf.pages[triggerNode.location];
    var uic918ticket = triggerNode.mimeType == "internal/uic9183" ? triggerNode.content : null;

    var reservations = parseTicket(page.text, uic918ticket);
    for (var i = 0; i < reservations.length; ++i) {
        reservations[i].reservedTicket.ticketToken = "aztecbin:" + Barcode.toBase64(triggerNode.content.rawData);
        if (triggerNode.result.length > 0) {
            reservations[i].reservedTicket.name = triggerNode.result[0].reservedTicket.name;
            reservations[i].underName = triggerNode.result[0].underName;
            if (reservations[i].reservedTicket.ticketedSeat) {
                reservations[i].reservedTicket.ticketedSeat.seatingType = triggerNode.result[0].reservedTicket.ticketedSeat.seatingType;
            }
        }
    }

    return reservations;
}

function parseCancellation(html) {
    var title = html.eval('//title')[0];
    var pnr = title.content.match(/Stornierungsbestätigung \(Auftrag (.*)\)/);
    if (!pnr)
        return null;
    var res = JsonLd.newTrainReservation();
    res.reservationNumber = pnr[1];
    res.reservationStatus = "ReservationCancelled";
    res.reservationFor = 1;
    return res;
}
