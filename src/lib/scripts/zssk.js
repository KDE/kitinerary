/*
   SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
   SPDX-License-Identifier: LGPL-2.0-or-later
*/


function parseDateTime(s)
{
    const DAY = 24 * 60 * 60 * 1000;
    const base = new Date(2009, 0, 1);
    let dt = new Date(s.substr(0, 4) * DAY + base.getTime());
    dt.setHours(s.substr(4, 2));
    dt.setMinutes(s.substr(6, 2));
    return dt;
}

function parseDomesticBarcodeLeg(payload, baseRes, i)
{
    if (payload.length <= i || payload[i] === "")
        return null;

    let res = JsonLd.clone(baseRes);
    while (payload[i] !== "") {
        if (dep = payload[i].match(/Z: (.*)/)) {
            res.reservationFor.departureStation.name = dep[1];
        } else if (arr = payload[i].match(/Do: (.*)/)) {
            res.reservationFor.arrivalStation.name = arr[1];
        } else if (seat = payload[i].match(/Vlak: (\d+)(?: Vozeň: (\d+) Miesto: (\d+))?/)) {
            res.reservationFor.trainNumber = seat[1];
            res.reservedTicket.ticketedSeat.seatSection = seat[2];
            res.reservedTicket.ticketedSeat.seatNumber = seat[3];
        } else if (km = payload[i].match(/Km: \d+(?: NO| ŽTO)?-(.*)-(?:(\d)\.tr\.|\*)/)) {
            res.reservationFor.trainName = km[1];
            res.reservedTicket.ticketedSeat.seatingType = km[2];
        }

        ++i;
    }
    ++i;
    return {res: res, i: i}
}

// see https://community.kde.org/KDE_PIM/KItinerary/ZSSK_Barcode
function parseDomesticBarcode(data) {
    const payload = ByteArray.decodeUtf8(ByteArray.inflate(data.slice(3))).split('\n');
    if (payload.length < 32) {
        return;
    }

    let res = JsonLd.newTrainReservation();
    res.reservationNumber = payload[1];
    res.reservationFor.departureStation.name = payload[6];
    res.reservationFor.arrivalStation.name = payload[7];
    const trainNum = payload[8].match(/(\d+)\[(?:(\d)\.tr\.|\*)]/);
    if (trainNum) {
        res.reservationFor.trainNumber = trainNum[1];
        res.reservedTicket.ticketedSeat.seatingType = trainNum[2];
    }
    res.reservationFor.departureTime = parseDateTime(payload[9]);
    res.underName.name = payload[11];
    res.programMembershipUsed.membershipNumber = payload[20];
    res.reservedTicket.name = payload[24];
    const tariff = payload[25].match(/(.*): \d/);
    if (tariff && payload[20]) {
        res.programMembershipUsed.name = tariff[1];
    }
    res.reservedTicket.validFrom = parseDateTime(payload[9]);
    res.reservedTicket.validUntil = parseDateTime(payload[10]);
    res.reservedTicket.ticketToken = 'aztecbin:' + ByteArray.toBase64(data);
    res.totalPrice = payload[3];
    res.priceCurrency = 'EUR';

    let i = 28;
    let reservations = [];
    while (true) {
        const r = parseDomesticBarcodeLeg(payload, res, i);
        if (!r)
            break;
        reservations.push(r.res);
        i = r.i;
    }

    // network ticket
    if (reservations.length === 1) {
        res = reservations[0];
        if (!res.reservationFor.departureStation.name || !res.reservationFor.arrivalStation.name) {
            res.reservedTicket.ticketNumber = res.reservationNumber;
            return res.reservedTicket;
        }
    }

    return reservations;
}

function parseDomesticPdf(pdf, node, triggerNode) {
    if (triggerNode.result[0]['@type'] == 'Ticket') {
        return triggerNode.result;
    }

    let reservations = [];
    const text = pdf.pages[triggerNode.location].text;
    let idx = 0;
    while (true) {
        const leg = text.substr(idx).match(/(\d{2}\.\d{2}.\d{2} +\d{2}:\d{2}) +.* (\S+)? -> .*? +(\S+ )?(\d{2}\.\d{2}\.\d{2} +\d{2}:\d{2})/);
        if (!leg)
            break;
        idx += leg.index + leg[0].length;
        let res = triggerNode.result[reservations.length];
        res.reservationFor.departureTime = JsonLd.toDateTime(leg[1], 'dd.MM.yy hh:mm', 'sk');
        res.reservationFor.departurePlatform = leg[2];
        res.reservationFor.arrivalTime = JsonLd.toDateTime(leg[4], 'dd.MM.yy hh:mm', 'sk');
        res.reservationFor.arrivalPlatform = leg[3];
        reservations.push(res);
    }

    return reservations;
}
