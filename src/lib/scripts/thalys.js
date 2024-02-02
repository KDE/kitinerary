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
    res.reservedTicket.ticketToken = "aztecbin:" + ByteArray.toBase64(ssb.rawData);
    res.reservedTicket.ticketNumber = ssb.tcn;
    return res;
}

function parseReservation(html, node) {
    var tokenElem = html.eval('//table[@class="qrcode"]//img')[0];
    var token = tokenElem.attribute("src").match(/barcode\/tAZTEC\/.*\/nBinary\/v(.*)\/barcode.gif/);
    var res = ExtractorEngine.extract(ByteArray.fromBase64(token[1])).result[0];

    var subtitle = html.eval('//table[@class="subtitle"]');
    var ref = subtitle[0].recursiveContent.match(/(\d{2}.\d{2}.\d{4})[\s\S]*([A-Z0-9]{6})/);
    res.reservationNumber = ref[2];

    var schedule = html.eval('//table[@class="schedule"]')[0].eval(".//tr");
    var stations = schedule[1].recursiveContent.match(/(.*)[\n\s]+\d{2}:\d{2}[\n\s](.*)/);
    res.reservationFor.departureStation.name = stations[1];
    res.reservationFor.arrivalStation.name = stations[2];

    var times = schedule[2].recursiveContent.match(/(\d{2}:\d{2})[\s\S]*(\d{2}:\d{2})/);
    res.reservationFor.departureTime = JsonLd.toDateTime(ref[1] + times[1], "dd/MM/yyyyhh:mm", "en");
    res.reservationFor.arrivalTime = JsonLd.toDateTime(ref[1] + times[2], "dd/MM/yyyyhh:mm", "en");

    var detailsElem = html.eval('//table[@class="detailtrain"]')[0];
    var details = detailsElem.recursiveContent.match(/(\d{4})[\n\s][\s\S]*?(\d{1})[\n\s][\s\S]*?(\d{1,2})[\n\s][\s\S]*?(\d{1,3})/);
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

    // determine departure day - this is in the ERA SSB code, but seems to occasionally be off by one day?
    const depDay = page.text.match(/\d\d\/\d{2}\/\d{4} +(\d{2})\/(\d{2})\/(\d{4})/);
    res.reservationFor.departureDay = depDay[3] + '-' + depDay[2] + '-' + depDay[1] + 'T00:00:00';

    const dep = page.textInRect(0.0, 0.15, 0.35, 0.3).match(/([\s\S]+)\n(?:DÉPART À|ABFAHRT|DEPARTURE AT|VERTREK OM)\n(\d\d:\d\d)/);
    res.reservationFor.departureStation.name = dep[1];
    res.reservationFor.departureTime = res.reservationFor.departureDay.substr(0, 11) + dep[2];

    const arr = page.textInRect(0.35, 0.15, 0.65, 0.3).match(/([\s\S]+)\n(?:ARRIVÉE À|ANKUNFT|ARRIVAL AT|AANKOMST OM)\n(\d\d:\d\d)/);
    res.reservationFor.arrivalStation.name = arr[1];
    res.reservationFor.arrivalTime = res.reservationFor.departureDay.substr(0, 11) + arr[2];

    const passenger = page.textInRect(0.0, 0.0, 0.35, 0.1).match(/(?:PASSAGER|FAHRGAST|PASSENGER|REIZIGER)\n(.*)\n/);
    res.underName.name = passenger[1];
    res.reservationNumber = page.textInRect(0.8, 0.0, 1.0,  0.2).match(/PNR\n([A-Z0-9]+)/)[1];

    // there is always a 17 digit number in ssb.type1OpenText - but what is that if no membership program is used??
    if (page.text.match(/(?:MEMBERSHIP |LOYALTYNUMMER)/))
        res.programMembershipUsed.membershipNumber = triggerNode.content.type1OpenText.match(/(\d{17})/)[1];

    const price = page.text.match(/(\d+ EUR)/);
    if (price)
        ExtractorEngine.extractPrice(price[1], res);
    return res;
}
