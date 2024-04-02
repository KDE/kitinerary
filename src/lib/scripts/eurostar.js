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
    return res;
}

function parsePdf(pdf, node, elb)
{
    const text = pdf.pages[elb.location].text;
    let res = elb.result[0];
    const dep = text.match(/From.*\n(.*\S)  +(\d\d:\d\d)/);
    res.reservationFor.departureStation.name = dep[1];
    res.reservationFor.departureTime = JsonLd.toDateTime(res.reservationFor.departureDay.substr(0, 10) + dep[2], "yyyy-MM-ddhh:mm", "en");
    const arr = text.match(/To.*\n(.*\S)  +(\d\d:\d\d)/);
    res.reservationFor.arrivalStation.name = arr[1];
    res.reservationFor.arrivalTime = JsonLd.toDateTime(res.reservationFor.departureDay.substr(0, 10) + arr[2], "yyyy-MM-ddhh:mm", "en");
    return res;
}

function parsePass(pass, node, elb)
{
    let res = elb.result[0];
    res.reservationFor.departureStation.name = pass.field["BoardingTime"].label;
    res.reservationFor.departureTime = JsonLd.toDateTime(res.reservationFor.departureDay.substr(0, 10) + pass.field["BoardingTime"].value, "yyyy-MM-ddhh:mm", "en");
    res.reservationFor.arrivalStation.name = pass.field["Arrivaltime"].label;
    res.reservationFor.arrivalTime = JsonLd.toDateTime(res.reservationFor.departureDay.substr(0, 10) + pass.field["Arrivaltime"].value, "yyyy-MM-ddhh:mm", "en");
    res.underName.name = pass.field["Passenger"].value;
    return res;
}
