/*
   SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
   SPDX-License-Identifier: LGPL-2.0-or-later
*/

function parseDateTime(value)
{
    const base = new Date(2017, 0, 1);
    return new Date(value * 1000 + base.getTime());
}

function parseUicStationCode(value, version)
{
    value &= 0xffffff;
    return version < 5 ? "uic:" + value : "hu:" + value;
}

// see https://community.kde.org/KDE_PIM/KItinerary/MAV_Barcode
// data starts at offset 20 in the header block, at which point both formats are structurally identical
function parseBarcodeCommon(res, data, version) {
    const view = new DataView(data);
    res.totalPrice = view.getFloat32(4);
    res.priceCurrency = 'HUF';

    const flags = view.getUInt8(8);
    let offset = 19; // header size

    // passenger block
    if (flags & 0x80) {
        res.underName.name = ByteArray.decodeUtf8(data.slice(offset, offset + 45));
        offset += 64;
    }

    // trip block
    if (flags & 0x01) {
        res.reservationFor.departureStation.identifier = parseUicStationCode(view.getUint32(offset + 3, false), version);
        res.reservationFor.departureStation.name = "" + (view.getUint32(offset + 3, false) & 0xffffff);
        res.reservationFor.arrivalStation.identifier = parseUicStationCode(view.getUint32(offset + 6 , false), version);
        res.reservationFor.arrivalStation.name = "" + (view.getUint32(offset + 6, false) & 0xffffff);
        res.reservedTicket.ticketedSeat.seatingType = ByteArray.decodeUtf8(data.slice(offset + 100, offset + 101));
        res.reservationFor.departureDay = parseDateTime(view.getUint32(offset + 102, false));
        res.reservedTicket.validFrom = res.reservationFor.departureDay;
        res.reservedTicket.validUntil = new Date(res.reservedTicket.validFrom);
        res.reservedTicket.validUntil.setMinutes((view.getUint32(offset + 105, false) & 0xffffff) + res.reservedTicket.validFrom.getMinutes());
        offset += 114
    }

    // supplement blocks
    for (let i = 0; i < view.getUInt8(9); ++i) {
        const supplementView = new DataView(data.slice(offset, offset + 23));
        res.reservedTicket.ticketedSeat.seatingType = ByteArray.decodeUtf8(data.slice(offset + 6, offset + 7));
        offset += 23;
    }

    // reservation blocks
    for (let i = 0; i < view.getUInt8(10); ++i) {
        const seatBlock = data.slice(offset);
        const seatView = new DataView(seatBlock);

        let seatOffset = 16 + (version < 6 ? 5 : 20);
        res.reservationFor.trainNumber = ByteArray.decodeUtf8(seatBlock.slice(16, seatOffset));
        if (seatView.getUInt8(seatOffset + 1)) {
            res.reservedTicket.ticketedSeat.seatSection = ByteArray.decodeUtf8(seatBlock.slice(seatOffset + 1, seatOffset + 4));
        }
        if (seatView.getUInt16(seatOffset + 4, false) !== 0) {
            res.reservedTicket.ticketedSeat.seatNumber = seatView.getUInt16(seatOffset + 4, false);
        }
        offset += version < 6 ? 57 : 72;
    }
}

function parseBarcode(data) {
    const barcodeView = new DataView(data);
    const version = barcodeView.getUInt8(0);

    let res = JsonLd.newTrainReservation();

    if (version == 4) {
        const inner = ByteArray.inflate(data.slice(2));
        const view = new DataView(inner);
        res.reservationNumber = ByteArray.decodeUtf8(inner.slice(0, 18));
        res.reservationFor.provider.identifier = "uic:" + view.getUint16(18, false);
        parseBarcodeCommon(res, inner.slice(20), version);
    } else if (version >= 5) { // version 5 and 6
        res.reservationNumber = ByteArray.decodeUtf8(data.slice(2, 20));
        res.reservationFor.provider.identifier = "uic:" + ByteArray.decodeUtf8(data.slice(20, 24));
        const inner = ByteArray.inflate(data.slice(24));
        parseBarcodeCommon(res, inner, version);
    } else {
        return;
    }

    // Can be PDF417 but those seem to render unreliably and result in failed scanning.
    // We use Aztec instead now which MAV scanners also support, and which is
    // generally more robust on mobile screens.
    res.reservedTicket.ticketToken = "aztecbin:" + ByteArray.toBase64(data);

    // Convert to a generic ticket when we don't have trip information
    if (!res.reservationFor.departureStation.name) {
        let ticket = res.reservedTicket;
        ticket.ticketNumber = res.reservationNumber;
        ticket.issuedBy = res.reservationFor.provider;
        ticket.underName = res.underName;
        ticket.priceCurrency = res.priceCurrency;
        ticket.totalPrice = res.totalPrice;
        return ticket;
    }

    return res;
}

function reservationFromBarcode(node) {
    if (node.result[0]['@type'] === 'TrainReservation')
        return JsonLd.clone(node.result[0]);
    let res = JsonLd.newTrainReservation();
    res.reservedTicket = JsonLd.clone(node.result[0]);
    res.reservationFor.provider = res.reservedTicket.issuedBy;
    res.reservedTicket.issuedBy = undefined;
    res.reservationNumber = res.reservedTicket.ticketNumber;
    res.priceCurrency = res.reservedTicket.priceCurrency;
    res.totalPrice = res.reservedTicket.totalPrice;
    return res;
}

function parseTicket(pdf, node, triggerNode) {
    let reservations = [];
    const text = pdf.pages[triggerNode.location].text;
    let idx = 0;
    while (true) {
        const trip = text.substr(idx).match(/(\d{2,4}\.\d{2}\.\d{2,4})\. *(\d{2}:\d{2}) *(.*) *-> *(.*) *(\d{2}:\d{2}) *(.*) *(\d)\./);
        if (!trip)
            break;
        idx += trip.index + trip[0].length;
        let res = reservationFromBarcode(triggerNode);
        res.reservationFor.departureStation.name = trip[3];
        res.reservationFor.arrivalStation.name = trip[4];
        res.reservationFor.departureTime = JsonLd.toDateTime(trip[1] + trip[2], ["yyyy.MM.ddhh:mm", "dd.MM.yyyyhh:mm"], "hu");
        res.reservationFor.arrivalTime = JsonLd.toDateTime(trip[1] + trip[5], ["yyyy.MM.ddhh:mm", "dd.MM.yyyyhh:mm"], "hu");
        res.reservationFor.trainNumber = trip[6];
        reservations.push(res);
    }
    if (reservations.length > 0)
        return reservations;

    // new/alternative layout
    idx = 0;
    while (true) {
        const trip = text.substr(idx).match(/  +(\S.*\S)  +(\d\d:\d\d)(?:  +(\S+))?\n(\d{2,4}\.\d{2}\.\d{2,4})\.  +(?:(\S+)  +)?(\d)\.\n  +(\S.*\S)  +(\d\d:\d\d)/);
        if (!trip)
            break;
        idx += trip.index + trip[0].length;
        let res = reservationFromBarcode(triggerNode);
        res.reservationFor.departureStation.name = trip[1];
        res.reservationFor.departureTime = JsonLd.toDateTime(trip[4] + trip[2], ["yyyy.MM.ddhh:mm", "dd.MM.yyyyhh:mm"], "hu");
        res.reservationFor.trainNumber = trip[3] ?? trip[5];
        res.reservedTicket.ticketedSeat.seatingType = trip[6];
        res.reservationFor.arrivalStation.name = trip[7];
        res.reservationFor.arrivalTime = JsonLd.toDateTime(trip[4] + trip[8], ["yyyy.MM.ddhh:mm", "dd.MM.yyyyhh:mm"], "hu");
        reservations.push(res);
    }

    return reservations;
}

function parseInternationalUic9183(uic9183, node)
{
    let res = node.result[0];
    const rct2 = uic9183.ticketLayout;
    const train = rct2.text(14, 1, 30, 1).match(/ZUGBINDUNG: (.*?) (.*)?/); // why is this in German??
    if (train[2] && train[2] != "null") {
        res.reservationFor.trainNumber = train[2] + ' ' + train[1];
    } else {
        res.reservationFor.trainNumber = train[1];
    }
    return res;
}

function parseInternationalTicket(pdf, node, triggerNode)
{
    const text = pdf.pages[triggerNode.location].text;
    let reservations = [];
    let idx = 0;
    while (true) {
        let leg = text.substr(idx).match(/(\d\d\.\d\d\.) (\d\d:\d\d) (.*) → (\d\d:\d\d) (.*?)  (.*)/);
        if (!leg)
            break;
        idx += leg.index + leg[0].length;

        let res = JsonLd.newTrainReservation();
        res.reservationFor.departureTime = JsonLd.toDateTime(leg[1] + leg[2], 'dd.MM.hh:mm', 'hu');
        res.reservationFor.arrivalTime = JsonLd.toDateTime(leg[1] + leg[4], 'dd.MM.hh:mm', 'hu');
        res.reservationFor.departureStation.name = leg[3];
        res.reservationFor.arrivalStation.name = leg[5];
        res.reservationFor.trainNumber = leg[6];
        res = JsonLd.apply(triggerNode.result[0], res);
        reservations.push(res);
    }
    return reservations;
}
