/*
   SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>
   SPDX-License-Identifier: LGPL-2.0-or-later
*/

function extractSmsTicket(content) {
    var res = JsonLd.newTrainReservation();

    var m = content.match(/PNR:(\d+),TRAIN:(.+?),DOJ:(.+?),TIME:(.+?),(.+?),(.+?) TO (.+?),(.*?)(\+\d)?,(.*?),/);
    res.reservationNumber = m[1];
    res.reservationFor.trainNumber = m[2];
    res.reservationFor.departureTime = JsonLd.toDateTime(m[3] + m[4], "dd-MM-yyyyhh:mm", "en");
    res.reservedTicket.ticketedSeat.seatingType = m[5];
    res.reservationFor.departureStation.name = m[6];
    res.reservationFor.departureStation.identifier = "ir:" + m[6];
    res.reservationFor.arrivalStation.name = m[7];
    res.reservationFor.arrivalStation.identifier = "ir:" + m[7];
    res.underName.name = m[8];
    res.reservedTicket.ticketedSeat.seatNumber = m[10];

    return res;
}

function extractQrTicket(text) {
    let res = JsonLd.newTrainReservation();
    res.reservedTicket.ticketToken = 'qrcode:' + text;
    res.reservationNumber = text.match(/PNR No.:(\d+),/)[1];
    res.reservationFor.trainNumber = text.match(/Train No.:(.*),/)[1];
    res.reservationFor.trainName = text.match(/Train Name:(.*),/)[1];
    const dep = text.match(/Boarding Station:(.*) - (.*),/);
    res.reservationFor.departureStation.name = dep[1];
    res.reservationFor.departureStation.identifier = 'ir:' + dep[2];
    const arr = text.match(/To:(.*) - (.*),/);
    res.reservationFor.arrivalStation.name = arr[1];
    res.reservationFor.arrivalStation.identifier = 'ir:' + arr[2];
    let dt = text.match(/Scheduled Departure:(\d{2}:\d{2}.*),/);
    if (!dt) {
        dt = text.match(/Scheduled Departure:(\d{2}-.+?-\d{4} \d{2}:\d{2}) *,/);
    }
    if (dt) {
        res.reservationFor.departureTime = JsonLd.toDateTime(dt[1], ["hh:mm dd-MMM-yyyy", "dd-MMM-yyyy hh:mm"], "en");
    } else {
        res.reservationFor.departureDay = JsonLd.toDateTime(text.match(/Date Of Journey:(.*),/)[1], "dd-MMM-yyyy", "en");
    }
    res.reservedTicket.ticketedSeat.seatingType = text.match(/Class:(.*),/)[1];

    let idx = 0;
    let reservations = [];
    while (true) {
        const pas = text.substr(idx).match(/Passenger Name:(.*),\n.*\n.*\n\s*(?:Status:(?:CNF|RAC)(.*)\/([^,\n]*))?/);
        if (!pas)
            break;
        idx += pas.index + pas[0].length;
        let r = JsonLd.clone(res);
        r.underName.name = pas[1];
        r.reservedTicket.ticketedSeat.seatSection = pas[2];
        r.reservedTicket.ticketedSeat.seatNumber = pas[3];
        reservations.push(r);
    }
    return reservations;
}

function extractPdfTicket(pdf, node, triggerNode) {
    const text = pdf.pages[triggerNode.location].text;
    const arr = JsonLd.toDateTime(text.match(/Arrival\* (.*)\n/)[1], "hh:mm dd-MMM-yyyy", "en");
    let res = triggerNode.result;
    for (let r of res)
        r.reservationFor.arrivalTime = arr;
    return res;
}
