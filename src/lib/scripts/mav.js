/*
   SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
   SPDX-License-Identifier: LGPL-2.0-or-later
*/

function parseDateTime(value)
{
    const base = new Date(2017, 0, 1);
    return new Date(value * 1000 + base.getTime());
}

function parseUicStationCode(value)
{
    value &= 0xffffff;
    return value < 1000000 ? undefined : "uic:" + value;
}

// see https://community.kde.org/KDE_PIM/KItinerary/MAV_Barcode
// data starts at offset 20 in the header block, at which point both formats are structurally identical
function parseBarcodeCommon(res, data) {
    const view = new DataView(data);
    const ticketType = view.getUInt8(8);
    const tripBlockOffset = ticketType == 0x81 ? 87 : 23;
    if (ticketType & 0x01) {
        res.reservationFor.departureStation.identifier = parseUicStationCode(view.getUint32(tripBlockOffset - 1, false));
        res.reservationFor.departureStation.name = "" + (view.getUint32(tripBlockOffset - 1, false) & 0xffffff);
        res.reservationFor.arrivalStation.identifier = parseUicStationCode(view.getUint32(tripBlockOffset + 2 , false));
        res.reservationFor.arrivalStation.name = "" + (view.getUint32(tripBlockOffset + 2, false) & 0xffffff);
        res.reservedTicket.ticketedSeat.seatingType = ByteArray.decodeUtf8(data.slice(tripBlockOffset + 96, tripBlockOffset + 97));
        res.reservationFor.departureDay = parseDateTime(view.getUint32(tripBlockOffset + 98, false));
        if (ticketType & 0x80) {
            res.underName.name = ByteArray.decodeUtf8(data.slice(19, 19 + 45));
        }
    }
    for (var i = 0; i < view.getUInt8(10); ++i) {
        const seatBlock = data.slice(data.byteLength - ((i+1) *57));
        const seatView = new DataView(seatBlock);
        res.reservationFor.trainNumber = ByteArray.decodeUtf8(seatBlock.slice(16, 16+5));
        if (seatView.getUInt8(22) == 0) { // surcharge block
            continue;
        }
        res.reservedTicket.ticketedSeat.seatSection = ByteArray.decodeUtf8(seatBlock.slice(22, 25));
        if (seatView.getUInt16(25, false) !== 0) {
            res.reservedTicket.ticketedSeat.seatNumber = seatView.getUInt16(25, false);
        }
    }
}

function parseBarcode(data) {
    var res = JsonLd.newTrainReservation();
    const inner = ByteArray.inflate(data.slice(2));
    const view = new DataView(inner);
    res.reservationNumber = ByteArray.decodeUtf8(inner.slice(0, 17));
    res.reservationFor.provider.identifier = "uic:" + view.getUint16(18, false);
    parseBarcodeCommon(res, inner.slice(20));
    res.reservedTicket.ticketToken = "pdf417bin:" + ByteArray.toBase64(data);
    return res;
}

function parseBarcodeAlternative(data)
{
    var res = JsonLd.newTrainReservation();
    res.reservationNumber = ByteArray.decodeUtf8(data.slice(2, 19));
    res.reservationFor.provider.identifier = "uic:" + ByteArray.decodeUtf8(data.slice(20, 24));

    const inner = ByteArray.inflate(data.slice(24));
    parseBarcodeCommon(res, inner);
    res.reservedTicket.ticketToken = "pdf417bin:" + ByteArray.toBase64(data);
    return res;
}

function parseTicket(pdf, node, triggerNode) {
    var reservations = new Array();
    const text = pdf.pages[triggerNode.location].text;
    var idx = 0;
    while (true) {
        var trip = text.substr(idx).match(/(\d{2,4}\.\d{2}\.\d{2,4})\. *(\d{2}:\d{2}) *(.*) *-> *(.*) *(\d{2}:\d{2}) *(.*) *(\d)\./);
        if (!trip) {
            break;
        }
        idx += trip.index + trip[0].length
        var res = JsonLd.clone(triggerNode.result[0]);
        res.reservationFor.departureStation.name = trip[3];
        res.reservationFor.arrivalStation.name = trip[4];
        res.reservationFor.departureTime = JsonLd.toDateTime(trip[1] + trip[2], ["yyyy.MM.ddhh:mm", "dd.MM.yyyyhh:mm"], "hu");
        res.reservationFor.arrivalTime = JsonLd.toDateTime(trip[1] + trip[5], ["yyyy.MM.ddhh:mm", "dd.MM.yyyyhh:mm"], "hu");
        res.reservationFor.trainNumber = trip[6];
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
