/*
    SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

function stripLeadingZeros(s) {
    return s.match(/^0*(.*)/)[1];
}

function parseBarcode(elb, node) {
    let res = JsonLd.newTrainReservation();
    res.reservationNumber = elb.pnr;
    res.reservationFor.departureStation.name = elb.segment1.departureStation;
    res.reservationFor.departureStation.identifier = 'benerail:' + elb.segment1.departureStation;
    res.reservationFor.arrivalStation.name = elb.segment1.arrivalStation;
    res.reservationFor.arrivalStation.identifier = 'benerail:' + elb.segment1.arrivalStation;
    res.reservationFor.departureDay = elb.segment1.departureDate(node.contextDateTime);
    res.reservationFor.trainName = 'Eurostar';
    res.reservationFor.trainNumber = stripLeadingZeros(elb.segment1.trainNumber);
    res.reservationFor.provider.identifier = 'uic:' + elb.futureUse.substr(1, 4);
    res.reservedTicket.ticketNumber = elb.tcnCode;
    res.reservedTicket.ticketToken = 'pdf417:' + elb.rawData;
    res.reservedTicket.ticketedSeat.seatSection = stripLeadingZeros(elb.segment1.coachNumber);
    res.reservedTicket.ticketedSeat.seatNumber = stripLeadingZeros(elb.segment1.seatNumber);
    res.reservedTicket.ticketedSeat.seatingType = elb.segment1.classOfTransport;
    res.reservedTicket.validFrom = elb.validFromDate();
    //res.reservedTicket.validUntil = elb.validUntilDate(); TODO this is set bus looks implausible?
    return res;
}

function parsePdf(pdf, node, elb) {
    const text = pdf.pages[elb.location].text;
    let res = elb.result[0];
    const times = text.match(/ (\d\d:\d\d) [\s\S]+ (\d\d:\d\d) /);
    const dep = text.match(/From.*\n(?:  +.*\n)?([ \S]\S.*?\S)  /);
    res.reservationFor.departureStation.name = dep[1];
    res.reservationFor.departureTime = JsonLd.toDateTime(times[1], "hh:mm", "en");
    const arr = text.match(/To.*\n(?:  +.*\n)?([ \S]\S.*?\S)(?:  |\n)/);
    res.reservationFor.arrivalStation.name = arr[1];
    res.reservationFor.arrivalTime = JsonLd.toDateTime(times[2], "hh:mm", "en");
    return res;
}

function parsePdfSSB(pdf, node, ssb) {
    const page = pdf.pages[ssb.location];
    const text = page.text;
    let res = ssb.result[0];
    let trip = text.match(/\d\d\/\d\d +(\d\d:\d\d) +(\d{4}).*\d\d\/\d\d +(\d\d:\d\d)/);
    if (trip) {
        res.reservationFor.departureTime = JsonLd.toDateTime(trip[1], "hh:mm", "en");
        res.reservationFor.arrivalTime = JsonLd.toDateTime(trip[3], "hh:mm", "en");
        res.reservationFor.trainNumber = trip[2];
        res.reservationNumber = text.match(/PNR: (\S{6})/)[1];
    } else {
        // alternative layout observed in NS-booked Eurostar-branded Thalys tickets since 2025
        const date = text.match(/\S+,? (\S+ \d{1,2}, \d{4}|\d{1,2}\.? \S+ \d{4})  .*\n/);
        const dep = page.textInRect(0, 0, 0.35, 1).match(/(?:FROM|DE|VON)\n([\s\S]+)\n *(?:D[EÉ]PART|ABFAHRT)/)[1];
        const arr = page.textInRect(0.35, 0, 1, 1).match(/(?:TO|À|NACH)\n([\s\S]+)\n *(?:ARRIV|ANKUNFT)/)[1];
        const time = text.match(/(?:D[EÉ]PART|ABFAHRT).*\n *(\d\d:\d\d) .*  +(\d\d:\d\d)/);
        res.reservationFor.departureTime = JsonLd.toDateTime(date[1] + ' ' + time[1], ["MMM d, yyyy hh:mm", "d MMMM yyyy hh:mm"], ["en", "fr"]);
        res.reservationFor.arrivalTime = JsonLd.toDateTime(date[1] + ' ' + time[2], ["MMM d, yyyy hh:mm", "d MMMM yyyy hh:mm"], ["en", "fr"]);
        res.reservationFor.departureStation.name = dep;
        res.reservationFor.arrivalStation.name = arr;
        const pas = text.match(/PNR\n *(\S.*\S)  +([A-Z0-9]{6})\n/);
        res.reservationNumber = pas[2];
        res.underName.name = pas[1];
        // pdf417 works just as well, but the vastly different format surprises people...
        res.reservedTicket.ticketToken = res.reservedTicket.ticketToken.replace(/^pdf417:/, "azteccode:");
    }
    return res;
}

function parsePass(pass, node, elb) {
    let res = elb.result[0];
    res.reservationFor.departureStation.name = pass.field["BoardingTime"].label;
    // Thalys has the wrong deprature day in their SSB code
    const dt = pass.field["Date"].value.match(/(\d+)\/(\d+)/);
    res.reservationFor.departureDay = res.reservationFor.departureDay.substr(0, 5) + dt[2] + '-' + dt[1];
    res.reservationFor.departureTime = JsonLd.toDateTime(res.reservationFor.departureDay.substr(0, 10) + pass.field["BoardingTime"].value, "yyyy-MM-ddhh:mm", "en");
    res.reservationFor.arrivalStation.name = pass.field["Arrivaltime"].label;
    res.reservationFor.arrivalTime = JsonLd.toDateTime(res.reservationFor.departureDay.substr(0, 10) + pass.field["Arrivaltime"].value, "yyyy-MM-ddhh:mm", "en");
    res.underName.name = pass.field["Passenger"].value;
    return res;
}
