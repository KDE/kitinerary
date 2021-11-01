/*
   SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
   SPDX-License-Identifier: LGPL-2.0-or-later
*/

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
        var res = JsonLd.newTrainReservation();
        res.reservationFor.departureStation.name = trip[3];
        res.reservationFor.arrivalStation.name = trip[4];
        res.reservationFor.departureTime = JsonLd.toDateTime(trip[1] + trip[2], ["yyyy.MM.ddhh:mm", "dd.MM.yyyyhh:mm"], "hu");
        res.reservationFor.arrivalTime = JsonLd.toDateTime(trip[1] + trip[5], ["yyyy.MM.ddhh:mm", "dd.MM.yyyyhh:mm"], "hu");
        res.reservationFor.trainNumber = trip[6];
        res.reservedTicket.ticketedSeat.seatingType = trip[7];
        res.reservedTicket.ticketToken = "pdf417bin:" + Barcode.toBase64(triggerNode.content);
        reservations.push(res);
    }
    return reservations;
}
