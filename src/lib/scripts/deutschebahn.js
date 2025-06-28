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
    var trainId = line.substr(idx).match(compact ? / +([^,]*?)(?=(,|$))/ : / +(.*?)(?=(  |Die |$))/);
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
    var platform = line.substr(idx).match(/^ {1,3}(.*?)(?=(  | IC| \d Fenster| \d Gang|$))/);
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
        var header = text.substr(pos).match(/Ihre Reiseverbindung[\S ]+(Hin|Rück|Einfache )[fF]ahrt am [0-9]{2}.[0-9]{2}.([0-9]{4}).*\n/);
        if (!header)
            break;
        var idx = header.index + header[0].length;
        var year = header[2];

        // determine ticket type
        var domesticHeader = text.substr(pos + idx).match(/  Reservierung(?: \/ Hinweise)?\n/);
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
        if (uic918ticket && header[1] !== "Rück") {
            reservations[0].reservationFor.departureStation = JsonLd.apply(JsonLd.toJson(uic918ticket.outboundDepartureStation), reservations[0].reservationFor.departureStation);
            reservations[reservations.length - 1].reservationFor.arrivalStation = JsonLd.apply(JsonLd.toJson(uic918ticket.outboundArrivalStation), reservations[reservations.length - 1].reservationFor.arrivalStation);
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
    const bookingRef = text.match(/(?:Auftragsnummer|Auftrag \(NVS\)):\s*([A-Z0-9]{6,9}|\d{12})\n/);
    const price = text.match(/(?:Summe|Gesamtpreis) *(\d+,\d{2} ?€)/)[1];
    for (var i = 0; i < reservations.length; ++i) {
        if (bookingRef) {
            reservations[i].reservationNumber = bookingRef[1];
        }

        if (reservations[i].reservationFor.trainNumber && reservations[i].reservationFor.trainNumber.startsWith("Bus")) {
            reservations[i] = JsonLd.trainToBusReservation(reservations[i]);
            reservations[i].reservedTicket.ticketedSeat = undefined;
        }
        ExtractorEngine.extractPrice(price, reservations[i]);
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
            break;
        idx += dep.index + dep[0].length;
        var arr = text.substr(idx).match(/  (\S.*\S) +an (\d\d:\d\d)  +(.*?)  +(.*)\n/);
        if (!arr)
            break;

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

    ExtractorEngine.extractPrice(text, reservations);
    return reservations;
}

function applyUic9183ToReservation(res, uicNode)
{
    const uicCode = uicNode.content;
    if (!res.reservationNumber || res.reservationNumber == uicCode.pnr)
        res.reservationNumber = uicCode.pnr;
    else
        res.reservedTicket.ticketNumber = uicCode.pnr;
    res.reservationFor.provider = JsonLd.toJson(uicCode.issuer);
    if (uicNode.result.length > 0 && uicNode.result[0].programMembershipUsed)
        res.programMembershipUsed = uicNode.result[0].programMembershipUsed;
    const bl = uicCode.block('0080BL');
    if (bl) {
        const sb = bl.findSubBlock('009');
        if (sb) {
            const bc = sb.content.match(/\d+-\d+-(.*)/)[1];
            switch (bc) {
                case "49":
                    res.programMembershipUsed.programName = "BahnCard 25";
                    break;
                case "19":
                case "78":
                    res.programMembershipUsed.programName = "BahnCard 50";
                    break;
            }
        }
    }
    res.reservedTicket.name = uicCode.name
    if (res.reservedTicket.ticketedSeat)
        res.reservedTicket.ticketedSeat.seatingType = uicCode.seatingType;
    res.underName = JsonLd.toJson(uicCode.person);
}

function parsePdf(pdf, node, triggerNode) {
    var page = pdf.pages[triggerNode.location];
    var uic918ticket = triggerNode.mimeType == "internal/uic9183" ? triggerNode.content : null;

    var reservations = parseTicket(page.text, uic918ticket);
    for (var i = 0; i < reservations.length; ++i) {
        reservations[i].reservedTicket.ticketToken = "aztecbin:" + ByteArray.toBase64(triggerNode.content.rawData);
        if (triggerNode.mimeType == "internal/uic9183") {
            applyUic9183ToReservation(reservations[i], triggerNode);
        } else if (triggerNode.mimeType == "internal/vdv") {
            reservations[i].reservationFor.provider.identifier = "vdv:" + triggerNode.content.operatorId;
            if (reservations[i].reservedTicket.ticketedSeat) {
                const c = triggerNode.content.seatingClass;
                reservations[i].reservedTicket.ticketedSeat.seatingType = (c == 1 || c == 3) ? '1' : '2';
            }
            reservations[i].underName = JsonLd.toJson(triggerNode.content.person);
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

function parseUic9183(code, node) {
    // Bahncard code
    if (code.ticketLayout && code.ticketLayout.type == "RCT2" && code.ticketLayout.text(0, 12, 40, 1).match(/BAHNCARD/i) && !code.block("U_FLEX")) {
        var bc = JsonLd.newObject("ProgramMembership");
        bc.programName = code.ticketLayout.text(1, 12, 40, 1);
        bc.membershipNumber = code.ticketLayout.text(14, 11, 16, 1);
        bc.member = JsonLd.toJson(code.person);
        bc.token = 'aztecbin:' + ByteArray.toBase64(code.rawData);
        bc.validFrom = JsonLd.readQDateTime(code, 'validFrom');
        bc.validUntil = JsonLd.readQDateTime(code, 'validUntil');
        return bc.programName != undefined ? bc : undefined;
    }

    // domestic ticket code
    const bl = code.block('0080BL');
    if (bl && code.outboundDepartureStation.name && code.outboundArrivalStation.name) {
        let res = JsonLd.newTrainReservation();
        res.reservedTicket = node.result[0];
        applyUic9183ToReservation(res, node);
        res.reservationFor.departureDay = JsonLd.toDateTime(bl.findSubBlock('031').content, 'dd.MM.yyyy', 'de');
        res.reservationFor.departureStation = JsonLd.toJson(code.outboundDepartureStation);
        res.reservationFor.arrivalStation = JsonLd.toJson(code.outboundArrivalStation);

        if (!bl.findSubBlock('017')) {
            return res;
        }

        let ret = JsonLd.newTrainReservation();
        ret.reservedTicket = node.result[0];
        applyUic9183ToReservation(ret, node);
        ret.reservationFor.departureDay = JsonLd.toDateTime(bl.findSubBlock('032').content, 'dd.MM.yyyy', 'de');
        ret.reservationFor.departureStation = JsonLd.toJson(code.returnDepartureStation);
        ret.reservationFor.arrivalStation = JsonLd.toJson(code.returnArrivalStation);
        return [res, ret];
    }
}

function parseEvent(event) {
    let res = JsonLd.newTrainReservation();
    const names = event.summary.match(/(.*) (?:->|➞) (.*)/);
    res.reservationFor.departureStation.name = names[1];
    res.reservationFor.departureTime = JsonLd.readQDateTime(event, 'dtStart');
    res.reservationFor.arrivalStation.name = names[2];
    res.reservationFor.arrivalTime = JsonLd.readQDateTime(event, 'dtEnd');

    // search for more details in the description
    let reservations = [];
    let idx = 0;
    while (true) {
        const trip = event.description.substr(idx).match(/(?:\[.*: \d .* (\S+), .* (\d+)\])?\n(?:(\S.*\S) \(.*\n)?.*(\d{2}:\d{2}) (.*?)(?:[-▷] (?:Gleis|platform|voie|Vía|Spor|Kolej|binario|Peron) (.*?))?(?: \((.*\d+)\))?\n.* (\d{2}:\d{2}) (.*?)(?:\n| [-▷] (?:Gleis|platform|voie|Vía|Spor|Kolej|binario|Peron) (.*)\n)/);
        if (!trip) {
            break;
        }
        idx += trip.index + trip[0].length;

        let res = JsonLd.newTrainReservation();
        const dtObj = JsonLd.readQDateTime(event, 'dtStart');
        const date = typeof dtObj === "string" ? dtObj.substr(0, 10) : dtObj['@value'].substr(0, 10);
        res.reservationFor.departureStation.name = trip[5];
        res.reservationFor.departureTime = JsonLd.toDateTime(date + trip[4], 'yyyy-MM-ddhh:mm', 'de');
        res.reservationFor.departurePlatform = trip[6];
        res.reservationFor.trainNumber = trip[3] ? trip[3] : trip[7];
        res.reservationFor.arrivalStation.name = trip[9];
        res.reservationFor.arrivalTime = JsonLd.toDateTime(date + trip[8], 'yyyy-MM-ddhh:mm', 'de');
        res.reservationFor.arrivalPlatform = trip[10];
        res.reservedTicket.ticketedSeat.seatSection = trip[1];
        res.reservedTicket.ticketedSeat.seatNumber = trip[2];

        if (res.reservationFor.trainNumber.match(/^Bus[ \d]/)) {
            res = JsonLd.trainToBusReservation(res);
        }

        reservations.push(res);
    }
    // recover full timezones for the begin/end
    if (reservations.length > 0) {
        reservations[0].reservationFor.departureTime = res.reservationFor.departureTime;
        reservations[reservations.length - 1].reservationFor.arrivalTime = res.reservationFor.arrivalTime;
        return reservations;
    }

    return res;
}

function parseReservationEvent(event) {
    let res = JsonLd.newTrainReservation();
    const names = event.summary.match(/(.*) ➞ (.*)/);
    res.reservationFor.departureStation.name = names[1];
    res.reservationFor.departureTime = JsonLd.readQDateTime(event, 'dtStart');
    res.reservationFor.arrivalStation.name = names[2];
    res.reservationFor.arrivalTime = JsonLd.readQDateTime(event, 'dtEnd');

    const trip = event.description.match(/.* ➞ .*\n\[.*: \d .* (\S+), .* (\d+)\]\n(.*) \(.*\n.* ab [^\d].* ▷ \S+ (\S.*)\n.* an ?.* ▷ \S+ (\S.*)\n/);
    res.reservationFor.departurePlatform = trip[4];
    res.reservationFor.trainNumber = trip[3];
    res.reservationFor.arrivalPlatform = trip[5];
    res.reservedTicket.ticketedSeat.seatSection = trip[1];
    res.reservedTicket.ticketedSeat.seatNumber = trip[2];

    return res;
}

// various create DB Regio ERA TLB in PLAI format...
function parseDBRegioBusUic(uic, node)
{
    let ticket = node.result[0];
    if (uic.ticketLayout.type != 'PLAI' || ticket.name)
        return;

    ticket.name = uic.ticketLayout.firstField.text;
    for (let f = uic.ticketLayout.firstField; f && !f.isNull; f = f.next) {
        const validFrom = f.text.match(/Erster Gültigkeitstag: (.*)/);
        if (validFrom) {
            ticket.validFrom = JsonLd.toDateTime(validFrom[1], 'dd.MM.yyyy', 'de');
        }
        const name = f.text.match(/Name Fahrtberechtigter: (.*)/);
        if (name) {
            ticket.underName = JsonLd.newObject('Person');
            ticket.underName.name = name[1];
        }
    }
    if (ticket.name.includes("Deutschland-Ticket") && !ticket.validUntil) {
        ticket.validUntil = JsonLd.clone(ticket.validFrom)
        ticket.validUntil.setMonth(ticket.validUntil.getMonth() + 1)
        ticket.validUntil.setHours(3)
    }
    return ticket;
}

function parseDBRegioNVUic(uic, node)
{
    let ticket = node.result[0];
    if (uic.ticketLayout.type != 'PLAI' || ticket.name)
        return;

    const text = uic.ticketLayout.text(1, 1, 33, 12);
    const data = text.match(/(.*) (\d).Kl\nVon (\d{2}\.\d{2}\.\d{4} \d{2}:\d{2})\nBis (\d{2}\.\d{2}\.\d{4} \d{2}:\d{2})\nVon (.*) \(.*\)\nNach (.*) \(.*\)\n/);
    ticket.name = data[1];
    ticket.ticketedSeat = { "@type": "Seat", seatingType: data[2] };
    ticket.validFrom = JsonLd.toDateTime(data[3], "dd.MM.yyyy hh:mm", "de");
    ticket.validUntil = JsonLd.toDateTime(data[4], "dd.MM.yyyy hh:mm", "de");

    let res = JsonLd.newTrainReservation();
    res.reservedTicket = ticket;
    res.reservationFor.departureStation.name = data[5];
    res.reservationFor.departureTime = ticket.validFrom;
    res.reservationFor.arrivalStation.name = data[6];
    return res;
}
