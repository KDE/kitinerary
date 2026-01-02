/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

function parseSsbBarcode(ssb, node)
{
    // vending machine bought and/or newer tickets claim version 2, but they aren't...
    if (ssb.version == 2) {
        let data = ssb.rawData.slice(0);
        let view = new Int8Array(data);
        view[0] = (view[0] & 0x0F) | 0x10;
        return ExtractorEngine.extract(data).result;
    }
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
    res.reservationFor.provider.identifier = "uic:" + ssb.issuerCode;

    res.reservedTicket.ticketToken = "aztecbin:" + ByteArray.toBase64(ssb.rawData);
    if (ssb.classOfTransport > 0 && ssb.classOfTransport <= 4)
        res.reservedTicket.ticketedSeat.seatingType = ssb.classOfTransport;

    if (ssb.coachNumber > 0) {
        res.reservedTicket.ticketedSeat.seatSection = ssb.coachNumber;
        res.reservedTicket.ticketedSeat.seatNumber = ssb.berthNumber + ssb.seatNumber;
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
    if (!trip)
        return parseMobilePdf(pdf, node, trigger);

    res.reservationFor.trainNumber = trip[6];
    res.reservationFor.departureTime = JsonLd.toDateTime(trip[4], "hh:mm", "en");
    res.reservationFor.arrivalTime = JsonLd.toDateTime(trip[5], "hh:mm", "en");
    res.reservationFor.departureStation.name = trip[1];
    res.reservationFor.arrivalStation.name = trip[2];
    return res;
}

function parseMobilePdf(pdf, node, ssb) {
    let res = ssb.result[0];
    const text = pdf.pages[ssb.location].textInRect(0.0, 0.0, 0.5, 0.5);
    const trip = text.match(/(\d{1,2}\.\d{1,2}\.\d{4})\n+ *(\S.*\S)  +(\S.*)\n+ *(\d\d:\d\d)  +(\d\d:\d\d)\n/);
    res.reservationFor.departureTime = JsonLd.toDateTime(trip[4], "hh:mm", "en");
    res.reservationFor.arrivalTime = JsonLd.toDateTime(trip[5], "hh:mm", "en");
    res.reservationFor.departureStation.name = trip[2];
    res.reservationFor.arrivalStation.name = trip[3];
    const seat = text.substr(trip.index + trip[0].length).match(/.*\n *([A-Z]+ \d+)  +(\d+).* (\d+)/);
    if (seat) {
        res.reservationFor.trainNumber = seat[1];
        res.reservedTicket.ticketedSeat.seatSection = seat[2];
        res.reservedTicket.ticketedSeat.seatNumber = seat[3];
    }
    return res;
}
