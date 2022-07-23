/*
   SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
   SPDX-License-Identifier: LGPL-2.0-or-later
*/

function parseDateTime(value)
{
    const base = new Date(2017, 0, 1);
    return new Date(value * 1000 + base.getTime());
}

// see https://community.kde.org/KDE_PIM/KItinerary/MAV_Barcode
function parseBarcode(data) {
    var res = JsonLd.newTrainReservation();
    const inner = ByteArray.inflate(data.slice(2));
    const view = new DataView(inner);
    res.reservationNumber = ByteArray.decodeUtf8(inner.slice(0, 17));
    res.reservationFor.provider.identifier = "uic:" + view.getUint16(18, false);
    const tripBlockOffset = view.getUInt8(28) == 0x81 ? 107 : 43;
    res.reservationFor.departureStation.identifier = "uic:" + (view.getUint32(tripBlockOffset - 1, false) & 0xffffff);
    res.reservationFor.departureStation.name = "" + (view.getUint32(tripBlockOffset - 1, false) & 0xffffff);
    res.reservationFor.arrivalStation.identifier = "uic:" + (view.getUint32(tripBlockOffset + 2 , false) & 0xffffff);
    res.reservationFor.arrivalStation.name = "" + (view.getUint32(tripBlockOffset + 2, false) & 0xffffff);
    res.reservedTicket.ticketedSeat.seatingType = ByteArray.decodeUtf8(inner.slice(tripBlockOffset + 96, tripBlockOffset + 97));
    res.reservationFor.departureDay = parseDateTime(view.getUint32(tripBlockOffset + 98, false));
    if (view.getUInt8(28) == 0x81) {
        res.underName.name = ByteArray.decodeUtf8(inner.slice(39, 39 + 45));
    }
    for (var i = 0; i < view.getUInt8(30); ++i) {
        const seatBlock = inner.slice(inner.byteLength - ((i+1) *57));
        const seatView = new DataView(seatBlock);
        res.reservationFor.trainNumber = ByteArray.decodeUtf8(seatBlock.slice(16, 16+5));
        if (seatView.getUInt8(22) == 0) { // surcharge block
            continue;
        }
        res.reservedTicket.ticketedSeat.seatSection = ByteArray.decodeUtf8(seatBlock.slice(22, 25));
        res.reservedTicket.ticketedSeat.seatNumber = seatView.getUInt16(25, false);
    }
    res.reservedTicket.ticketToken = "pdf417bin:" + ByteArray.toBase64(data);
    return res;
}

function parseBarcodeAlternative(data)
{
    var res = JsonLd.newTrainReservation();
    res.reservationNumber = ByteArray.decodeUtf8(data.slice(2, 19));
    res.reservationFor.provider.identifier = "uic:" + ByteArray.decodeUtf8(data.slice(20, 24));

    const inner = ByteArray.inflate(data.slice(24));
    const header2 = new DataView(inner.slice(0, 19));
    if (header2.getUInt8(8) == 0x81) {
        res.underName.name = ByteArray.decodeUtf8(inner.slice(19, 19 + 45));
    }
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
