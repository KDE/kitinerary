/*
   SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
   SPDX-License-Identifier: LGPL-2.0-or-later
*/

function parseDateTime(value)
{
    const base = new Date(2017, 0, 1);
    return new Date(value * 1000 + base.getTime());
}

function parseBarcode(data) {
    var res = JsonLd.newTrainReservation();
    // see https://community.kde.org/KDE_PIM/KItinerary/MAV_Barcode
    const inner = ByteArray.inflate(data.slice(2));
    const view = new DataView(inner);
    if (view.getUInt8(28) == 0x81) {
        res.reservationFor.departureStation.identifier = "uic:" + (view.getUint32(106, false) & 0xffffff);
        res.reservationFor.departureStation.name = "" + (view.getUint32(106, false) & 0xffffff);
        res.reservationFor.arrivalStation.identifier = "uic:" + (view.getUint32(109, false) & 0xffffff);
        res.reservationFor.arrivalStation.name = "" + (view.getUint32(109, false) & 0xffffff);
        res.reservationFor.provider.identifier = "uic:" + view.getUint16(18, false);
        res.reservationFor.departureDay = parseDateTime(view.getUint32(205, false));
        res.reservationNumber = ByteArray.decodeUtf8(inner.slice(0, 17));
        res.underName.name = ByteArray.decodeUtf8(inner.slice(39, 39 + 45));
        res.reservedTicket.ticketedSeat.seatingType = ByteArray.decodeUtf8(inner.slice(203, 204));
    }
    for (var i = 0; i < view.getUInt8(30); ++i) {
        const seatBlock = inner.slice(inner.byteLength - ((i+1) *57));
        const seatView = new DataView(seatBlock);
        if (seatView.getUInt8(22) == 0) { // surcharge block
            continue;
        }
        res.reservedTicket.ticketedSeat.seatSection = ByteArray.decodeUtf8(seatBlock.slice(22, 25));
        res.reservedTicket.ticketedSeat.seatNumber = seatView.getUInt16(25, false);
    }
    res.reservedTicket.ticketToken = "pdf417bin:" + ByteArray.toBase64(data);
    return res;
}

function parseTicket(pdf, node, triggerNode) {
    var reservations = new Array();
    const text = pdf.pages[triggerNode.location].text;
    var idx = 0;
    while (true) {
        var trip = text.substr(idx).match(/(\d{4}\.\d{2}\.\d{2})\. *(\d{2}:\d{2}) *(.*) *-> *(.*) *(\d{2}:\d{2}) *(.*) *(\d)\./);
        if (!trip) {
            break;
        }
        idx += trip.index + trip[0].length
        var res = JsonLd.clone(triggerNode.result[0]);
        res.reservationFor.departureStation.name = trip[3];
        res.reservationFor.arrivalStation.name = trip[4];
        res.reservationFor.departureTime = JsonLd.toDateTime(trip[1] + trip[2], "yyyy.MM.ddhh:mm", "hu");
        res.reservationFor.arrivalTime = JsonLd.toDateTime(trip[1] + trip[5], "yyyy.MM.ddhh:mm", "hu");
        res.reservationFor.trainNumber = trip[6];
        res.reservedTicket.ticketedSeat.seatingType = trip[7];
        reservations.push(res);
    }
    return reservations;
}
