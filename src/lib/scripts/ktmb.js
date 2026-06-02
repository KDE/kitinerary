/*
   SPDX-FileCopyrightText: 2026 Ajinkya Dahale <dahale.a.p@gmail.com>
   SPDX-License-Identifier: LGPL-2.0-or-later
*/

function extractQrTicket(text) {
    let res = JsonLd.newTrainReservation();

    res.reservedTicket.ticketToken = 'qrcode:' + text;

    var extrData = text.match(/([0-9]*),[a-f0-9-]{36},(T[0-9]{12}),([^,]*),([^,]*),([^,]*),[^,]*,[^,]*,(.+)/);

    if (parseInt(extrData[1]) != 1) {
        // TODO: multi passenger support?
        return [];
    }

    res.reservationNumber = extrData[2];
    res.reservationFor.trainNumber = extrData[3];
    res.reservationFor.departureTime = JsonLd.toDateTime(extrData[6], "dd MMM yyyy hh:mm:ss ap", "en");
    res.reservedTicket.ticketedSeat.seatSection = extrData[4];
    res.reservedTicket.ticketedSeat.seatNumber = extrData[5];

    return res;
}

function extractPdfTicketWithQr(pdf, node, triggerNode) {
    let res = triggerNode.result;
    const text = pdf.pages[triggerNode.location].text;

    var extrData = text.match(/\s+(.+) - (.+)\n\s*Berlepas \/ Departure :\s*Tiba \/ Arrival :\n\n\s+(\d{2}\/\d{2}\/\d{4} \d{2}:\d{2} [AP]M)\s+(\d{2}\/\d{2}\/\d{4} \d{2}:\d{2} [AP]M)\n\n\s+(.+)\n\s+(.+) - Adult\n\s+(\S+)\s+Coach\/Seat\n\s+MYR ([0-9.]+) \/ .+\S\s\s+(.+)\/(.+)\n\s+(K\d{12})\*\*(T\d{12}) \(Single\)\s+Online\s+(.+)/);

    // TODO: not supposed to be there for just "adults"
    // This may include other classes such as "senior" or "student"
    for (let r of res) {
        r.reservationFor.departureStation.name = extrData[1];
        r.reservationFor.arrivalStation.name = extrData[2];
        r.reservationFor.departureTime = JsonLd.toDateTime(extrData[3], "dd/MM/yyyy hh:mm ap", "en");
        r.reservationFor.arrivalTime = JsonLd.toDateTime(extrData[4], "dd/MM/yyyy hh:mm ap", "en");
        r.reservationFor.trainName = extrData[5];
        r.underName.name = extrData[6];
        // r.underName.identifier = extrData[7]; // FIXME: Confirm
        r.totalPrice = parseFloat(extrData[8]);
        r.priceCurrency = 'MYR';
        r.reservedTicket.ticketedSeat.seatSection = extrData[9];
        r.reservedTicket.ticketedSeat.seatNumber = extrData[10];
        // There are two numbers: booking, which is common for all, and ticket, unique for each passenger
        r.reservationNumber = extrData[12];
    }

    return res;
}

function extractTextTicket(text) {
    let res = JsonLd.newTrainReservation();

    // TODO: not supposed to be there for just "adults"
    // This may include other classes such as "senior" or "student"
    var extrData = text.match(/\s+(.+) - (.+)\n\s*Berlepas \/ Departure :\s*Tiba \/ Arrival :\n\n\s+(\d{2}\/\d{2}\/\d{4} \d{2}:\d{2} [AP]M)\s+(\d{2}\/\d{2}\/\d{4} \d{2}:\d{2} [AP]M)\n\n\s+(.+)\n\s+(.+) - Adult\n\s+(\S+)\s+Coach\/Seat\n\s+MYR ([0-9.]+) \/ .+\S\s\s+(.+)\/(.+)\n\s+(K\d{12})\*\*(T\d{12}) \(Single\)\s+Online\s+(.+)/);

    res.reservationFor.departureStation.name = extrData[1];
    res.reservationFor.arrivalStation.name = extrData[2];
    res.reservationFor.departureTime = JsonLd.toDateTime(extrData[3], "dd/MM/yyyy hh:mm ap", "en");
    res.reservationFor.arrivalTime = JsonLd.toDateTime(extrData[4], "dd/MM/yyyy hh:mm ap", "en");
    res.reservationFor.trainName = extrData[5];
    res.underName.name = extrData[6];
    // res.underName.identifier = extrData[7]; // FIXME: Confirm
    res.totalPrice = parseFloat(extrData[8]);
    res.priceCurrency = 'MYR';
    res.reservedTicket.ticketedSeat.seatSection = extrData[9];
    res.reservedTicket.ticketedSeat.seatNumber = extrData[10];
    // There are two numbers: booking, which is common for all, and ticket, unique for each passenger
    res.reservationNumber = extrData[12];

    return res;
}

function extractPdfTicketWithoutQr(pdf, node, triggerNode) {
    let res = triggerNode.result;
    const text = pdf.pages[triggerNode.location].text;

    var extrData = text.match(/\s+(.+) - (.+)\n\s*Berlepas \/ Departure :\s*Tiba \/ Arrival :\n\n\s+(\d{2}\/\d{2}\/\d{4} \d{2}:\d{2} [AP]M)\s+(\d{2}\/\d{2}\/\d{4} \d{2}:\d{2} [AP]M)\n\n\s+(.+)\n\s+(.+) - Adult\n\s+(\S+)\s+Coach\/Seat\n\s+MYR ([0-9.]+) \/ .+\S\s\s+(.+)\/(.+)\n\s+(K\d{12})\*\*(T\d{12}) \(Single\)\s+Online\s+(.+)/);

    // TODO: not supposed to be there for just "adults"
    // This may include other classes such as "senior" or "student"
    for (let r of res) {
        r.reservationFor.departureStation.name = extrData[1];
        r.reservationFor.arrivalStation.name = extrData[2];
        r.reservationFor.departureTime = JsonLd.toDateTime(extrData[3], "dd/MM/yyyy hh:mm ap", "en");
        r.reservationFor.arrivalTime = JsonLd.toDateTime(extrData[4], "dd/MM/yyyy hh:mm ap", "en");
        r.reservationFor.trainName = extrData[5];
        r.underName.name = extrData[6];
        // r.underName.identifier = extrData[7]; // FIXME: Confirm
        r.totalPrice = parseFloat(extrData[8]);
        r.priceCurrency = 'MYR';
        r.reservedTicket.ticketedSeat.seatSection = extrData[9];
        r.reservedTicket.ticketedSeat.seatNumber = extrData[10];
        // There are two numbers: booking, which is common for all, and ticket, unique for each passenger
        r.reservationNumber = extrData[12];
    }

    return res;
}
