/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

function parseSsbTicket(ssb, node) {
    var res = JsonLd.newTrainReservation();
    if (ssb.version != 3 || ssb.ticketTypeCode != 1) {
        return res;
    }
    res.reservationFor.departureStation.name = ssb.type1DepartureStationAlpha;
    res.reservationFor.departureStation.identifier = "benerail:" + ssb.type1DepartureStationAlpha;
    res.reservationFor.arrivalStation.name = ssb.type1ArrivalStationAlpha;
    res.reservationFor.arrivalStation.identifier = "benerail:" + ssb.type1ArrivalStationAlpha;
    res.reservationFor.departureDay = ssb.type1DepartureDay(node.contextDateTime)
    res.reservationFor.trainNumber = "THA " + ssb.type1TrainNumber.trim();
    res.reservationFor.provider.identifier = "uic:" + ssb.issuerCode;
    res.reservedTicket.ticketedSeat.seatingType = ssb.classOfTravel;
    res.reservedTicket.ticketedSeat.seatSection = ssb.type1CoachNumber;
    res.reservedTicket.ticketedSeat.seatNumber = ssb.type1SeatNumber;
    res.reservationNumber = ssb.tcn;
    res.reservedTicket.ticketToken = "aztecbin:" + ByteArray.toBase64(ssb.rawData);
    return res;
}

function parseReservation(html, node) {
    var tokenElem = html.eval('//table[@class="qrcode"]//img')[0];
    var token = tokenElem.attribute("src").match(/barcode\/tAZTEC\/.*\/nBinary\/v(.*)\/barcode.gif/);
    var res = parseSsbTicket(Barcode.decodeEraSsbTicket(ByteArray.fromBase64(token[1])), node);

    var subtitle = html.eval('//table[@class="subtitle"]');
    var ref = subtitle[0].recursiveContent.match(/(\d{2}.\d{2}.\d{4})[\s\S]*([A-Z0-9]{6})/);
    res.reservationNumber = ref[2];

    var schedule = html.eval('//table[@class="schedule"]')[0].eval(".//tr");
    var stations = schedule[1].recursiveContent.match(/(.*)\n.*\n(.*)/);
    res.reservationFor.departureStation.name = stations[1];
    res.reservationFor.arrivalStation.name = stations[2];

    var times = schedule[2].recursiveContent.match(/(\d{2}:\d{2})[\s\S]*(\d{2}:\d{2})/);
    res.reservationFor.departureTime = JsonLd.toDateTime(ref[1] + times[1], "dd/MM/yyyyhh:mm", "en");
    res.reservationFor.arrivalTime = JsonLd.toDateTime(ref[1] + times[2], "dd/MM/yyyyhh:mm", "en");

    var detailsElem = html.eval('//table[@class="detailtrain"]')[0];
    var details = detailsElem.recursiveContent.match(/(\d{4})\n[\s\S]*?(\d{1})\n[\s\S]*?(\d{1,2})\n[\s\S]*?(\d{1,3})/);
    res.reservationFor.trainNumber = "THA " + details[1];
    res.reservedTicket.ticketedSeat.seatingType = details[2];
    res.reservedTicket.ticketedSeat.seatSection = details[3];
    res.reservedTicket.ticketedSeat.seatNumber = details[4];

    var passengerElem = html.eval('//table[@class="passengername"]')[0];
    var name = passengerElem.recursiveContent.match(/\n(.*)/);
    res.underName.name = name[1];
    return res;
}

function parsePdfTicket(pdf, node, triggerNode)
{
    const page = pdf.pages[triggerNode.location];
    var res = triggerNode.result[0];

    const dep = page.textInRect(0.0, 0.15, 0.35, 0.3).match(/([\s\S]+)\nDÉPART À\n(\d\d:\d\d)/);
    res.reservationFor.departureStation.name = dep[1];
    res.reservationFor.departureTime = res.reservationFor.departureDay.substr(0, 11) + dep[2];

    const arr = page.textInRect(0.35, 0.15, 0.65, 0.3).match(/([\s\S]+)\nARRIVÉE À\n(\d\d:\d\d)/);
    console.log(page.textInRect(0.35, 0.15, 0.65, 0.3));
    res.reservationFor.arrivalStation.name = arr[1];
    res.reservationFor.arrivalTime = res.reservationFor.departureDay.substr(0, 11) + arr[2];

    const passenger = page.text.match(/PASSAGER\n(.*)\n/);
    res.underName.name = passenger[1];

    return res;
}
