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

// see https://community.kde.org/KDE_PIM/KItinerary/ZSSK_Barcode
function parseDomesticBarcode(data) {
    const payload = ByteArray.decodeUtf8(ByteArray.inflate(data.slice(3))).split('\n');
    if (payload.length > 34 || payload.length < 32) {
        return;
    }

    let res = JsonLd.newTrainReservation();
    res.reservationNumber = payload[1];
    res.reservationFor.departureStation.name = payload[6];
    res.reservationFor.arrivalStation.name = payload[7];
    const trainNum = payload[8].match(/(\d+)\[(?:(\d)\.tr\.|\*)]/);
    res.reservationFor.trainNumber = trainNum[1];
    res.reservedTicket.ticketedSeat.seatingType = trainNum[2];
    res.reservationFor.departureTime = parseDateTime(payload[9]);
    res.underName.name = payload[11];
    res.programMembershipUsed.membershipNumber = payload[20];
    res.reservedTicket.name = payload[24];
    const tariff = payload[25].match(/(.*): \d/);
    if (tariff && payload[20]) {
        res.programMembershipUsed.name = tariff[1];
    }
    if (payload.length == 34) {
        const km = payload[30].match(/Km: \d+ (?:NO|ŽTO)-(.*)-(?:\d\.tr\.|\*)/);
        res.reservationFor.trainName = km[1];
    }
    const seat = payload[payload.length == 34 ? 31 : 30].match(/Vlak: \d+ Vozeň: (\d+) Miesto: (\d+)/);
    if (seat) {
        res.reservedTicket.ticketedSeat.seatSection = seat[1];
        res.reservedTicket.ticketedSeat.seatNumber = seat[2];
    }
    res.reservedTicket.validFrom = parseDateTime(payload[9]);
    res.reservedTicket.validUntil = parseDateTime(payload[10]);
    res.reservedTicket.ticketToken = 'aztecbin:' + ByteArray.toBase64(data);
    res.totalPrice = payload[3];
    res.priceCurrency = 'EUR';

    // network ticket
    if (!res.reservationFor.departureStation.name || !res.reservationFor.arrivalStation.name) {
        res.reservedTicket.ticketNumber = res.reservationNumber;
        return res.reservedTicket;
    }

    return res;
}

function parseDomesticPdf(pdf, node, triggerNode) {
    const text = pdf.pages[triggerNode.location].text;
    // TODO multi-leg support?
    const leg = text.match(/\d{2}\.\d{2}.\d{2} +\d{2}:\d{2} +.*  -> .* +(\d{2}\.\d{2}\.\d{2} +\d{2}:\d{2})/);
    let res = triggerNode.result[0];
    res.reservationFor.arrivalTime = JsonLd.toDateTime(leg[1], 'dd.MM.yy hh:mm', 'sk');
    return res;
}
