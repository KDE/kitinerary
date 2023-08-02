/*
   SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
   SPDX-License-Identifier: LGPL-2.0-or-later
*/

function readRSP6String(data, start, len) {
    let result = "";
    for (let i = 0; i < len; ++i) {
        const c = data.readNumberMSB(i * 6 + start, 6) + 32;
        result += String.fromCharCode(c);
    }
    return result.trim() ? result : null;
}

// TODO which timezone is this?
function readRSP6DateTime(data, start) {
    const days = data.readNumberMSB(start, 14);
    const secs = data.readNumberMSB(start + 14, 11);
    let dt = new Date(1997, 0, 1);
    dt.setTime(dt.getTime() + days * 24 * 60 * 60 * 1000 + secs * 1000 * 60);
    return dt;
}

// see https://git.eta.st/eta/rsp6-decoder/src/branch/master/spec.pdf
function parseRSP6(text) {
    const rsp6Data = ByteArray.decodeRsp6Ticket(text);
    const rsp6 = ByteArray.toBitArray(rsp6Data);

    let res = JsonLd.newTrainReservation();
    res.reservationNumber = text.substr(13, 2) + readRSP6String(rsp6, 8, 9);
    res.reservationFor.departureStation.name = readRSP6String(rsp6, 109, 4);
    res.reservationFor.departureStation.identifier = 'nlc:' + readRSP6String(rsp6, 109, 4);
    res.reservationFor.arrivalStation.name = readRSP6String(rsp6, 133, 4);
    res.reservationFor.arrivalStation.identifier = 'nlc:' + readRSP6String(rsp6, 133, 4);

    const departureTime = readRSP6DateTime(rsp6, 211);
    const departureTimeFlag = rsp6.readNumberMSB(236, 2);
    if (departureTimeFlag > 0) {
        res.reservationFor.departureTime = departureTime;
    } else {
        res.reservationFor.departureDay = departureTime;
        res.reservedTicket.validFrom = departureTime;
    }

    res.underName.name = readRSP6String(rsp6, 255, 12);

    console.log("version", rsp6.readNumberMSB(68, 4));
    console.log("ticket type", readRSP6String(rsp6, 73, 3));
    console.log("fare label", readRSP6String(rsp6, 91, 3));
    console.log("retailer", readRSP6String(rsp6,157, 4));
    console.log("coupon type", rsp6.readNumberMSB(182, 2));
    console.log("discount code", rsp6.readNumberMSB(184, 10));
    console.log("route code", rsp6.readNumberMSB(194, 17));
    console.log("passenger ID", rsp6.readNumberMSB(238, 17));
    console.log("passenger gender", rsp6.readNumberMSB(327, 2));
    console.log("restriction code", readRSP6String(329, 7));
    console.log("limited duration code", rsp6.readNumberMSB(379, 4));

    let offset = 0;
    if (rsp6.readNumberMSB(384, 1)) {
        console.log("purchase time", readRSP6DateTime(rsp6, 390));
        console.log("purchase ref", readRSP6String(rsp6, 449, 8));
        res.totalPrice = rsp6.readNumberMSB(415, 21) / 100.0;
        res.priceCurrency = 'GBP';
        const daysOfValidity = rsp6.readNumberMSB(497, 9);
        if (departureTimeFlag == 0) {
            let dt = new Date(departureTime);
            dt.setTime(dt.getTime() + Math.max(1, daysOfValidity) * 24 * 60 * 60 * 1000);
            if (daysOfValidity == 0) {
                dt.setTime(dt.getTime() - 1000);
            }
            res.reservedTicket.validUntil = dt;
        }
        offset += 122;
    }

    // TODO handle more than one reservation properly
    for (let i = 0; i < rsp6.readNumberMSB(386, 4); ++i) {
        res.reservationFor.trainNumber = readRSP6String(rsp6, 390 + offset, 2) + rsp6.readNumberMSB(402 + offset, 14);
        res.reservedTicket.ticketedSeat.seatSection = readRSP6String(rsp6, 416 + offset, 1);
        if (res.reservedTicket.ticketedSeat.seatSection == '*') {
            res.reservedTicket.ticketedSeat.seatSection = null;
        }
        const seatLetter = readRSP6String(rsp6, 422 + offset, 1);
        const seatNum = rsp6.readNumberMSB(428 + offset, 7);
        if (seatLetter || seatNum) {
            res.reservedTicket.ticketedSeat.seatNumber = (seatLetter ? seatLetter : "") + (seatNum ? seatNum : "");
        }
        offset += 45;
    }

    if (rsp6.readNumberMSB(385, 1)) {
        console.log("free text", readRSP6String(390 + offset, 51));
    }

    res.reservedTicket.ticketToken = 'aztec:' + text;
    return res;
}

function parseTicket(pdf, node, triggerNode) {
    const text = pdf.pages[triggerNode.location].text;
    var res = triggerNode.result[0];
    const header = text.match(/= +(\d{2}[ -][A-Z][a-z]{2}[ -]\d{4}) +(?:Out: |Ret: )?([A-Z]{3}) ?- ?([A-Z]{3})\n(.*)  +(.*)/);
    const date = header[1].replace(/-/g, ' ');
    const itinerary = text.match(/Itinerary.*\n +(.*)\n +(\d{2}:\d{2})\n +([\S\s]*?)\n +(\d{2}:\d{2})\n +(.*)/);

    res.reservationFor.departureStation.identifier = 'uk:' + header[2];
    res.reservationFor.arrivalStation.identifier = 'uk:' + header[3];
    if (itinerary) {
        res.reservationFor.departureStation.name = itinerary[1];
        res.reservationFor.departureTime = JsonLd.toDateTime(date + ' ' + itinerary[2], 'dd MMM yyyy hh:mm', 'en');
        res.reservationFor.arrivalStation.name = itinerary[5];
        res.reservationFor.arrivalTime = JsonLd.toDateTime(date + ' ' + itinerary[4], 'dd MMM yyyy hh:mm', 'en');
        res.reservationFor.trainName = itinerary[3].replace(/(.*?)(?:  .*)?(?:\n +|$)/g, '$1 ');
    } else {
        // unbound ticket
        res.reservationFor.departureStation.name = header[4];
        res.reservationFor.arrivalStation.name = header[5];
        res.reservationFor.departureDay = JsonLd.toDateTime(date, 'dd MMM yyyy', 'en');
    }

    return res;
}
