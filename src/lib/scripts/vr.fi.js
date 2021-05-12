/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

function parseSsbBarcode(ssb, node)
{
    if (ssb.version != 1) {
        return;
    }
    var res = JsonLd.newTrainReservation();
    if (ssb.trainNumber > 0) {
        res.reservationFor.trainNumber = ssb.trainNumber;
    }
    res.reservationFor.departureDay = ssb.firstDayOfValidity(node.contextDateTime);
    res.reservationFor.departureTime = ssb.departureTime(node.contextDateTime);
    res.reservationFor.departureStation.name = ssb.departureStationAlpha;
    res.reservationFor.arrivalStation.name = ssb.arrivalStationAlpha;
    // for station codes see: https://rata.digitraffic.fi/api/v1/metadata/stations
    res.reservationFor.departureStation.identifier = "vrfi:" + ssb.departureStationAlpha;
    res.reservationFor.arrivalStation.identifier = "vrfi:" + ssb.arrivalStationAlpha;

    res.reservedTicket.ticketToken = "aztectbin:" + Barcode.toBase64(ssb.rawData);
    res.reservedTicket.ticketedSeat.seatingType = ssb.classOfTransport;

    if (ssb.coachNumber > 0) {
        res.reservedTicket.ticketedSeat.seatSection = ssb.coachNumber;
        res.reservedTicket.ticketedSeat.seatNumber = ssb.seatNumber;
    }

    res.reservationNumber = ssb.reservationReference + "";
    return res;
}

function parseTicket(pdf, node, trigger) {
    var res = trigger.result[0];
    if (trigger.content.trainNumber == 0)
        return; // TODO this misses bus legs!

    var text = pdf.pages[trigger.location].text;
    var trip = text.match("(.*) - (.*)\n.*(\\d{4}).*?(\\d{2}:\\d{2}).*?(\\d{2}:\\d{2})\n(.*?" + trigger.content.trainNumber + ")");
    res.reservationFor.trainNumber = trip[6];

    res.reservationFor.departureTime = JsonLd.toDateTime(trip[4], "hh:mm", "en");
    res.reservationFor.arrivalTime = JsonLd.toDateTime(trip[5], "hh:mm", "en");
    res.reservationFor.departureStation.name = trip[1];
    res.reservationFor.arrivalStation.name = trip[2];
    return res;
}
