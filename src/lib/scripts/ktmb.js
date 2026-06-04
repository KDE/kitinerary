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

function extractPdfTicketWithRects(pdf, node, triggerNode) {
    var res = triggerNode.result;
    if (!res || res.length == 0) {
        res = [JsonLd.newTrainReservation()];
    }

    const textLeft = pdf.pages[0].textInRect(0.0, 0.0, 0.56, 1.0);
    const textRight = pdf.pages[0].textInRect(0.56, 0.0, 0.67, 0.50);

    // TODO: multi passenger support?
    // TODO: not supposed to be there for just "adults"
    // This may include other classes such as "senior" or "student"
    var extrDataLeft = textLeft.match(/\s*(.+) - (.+)\n+\s*Berlepas\s*\/\s*Departure\s*:\s*Tiba\s*\/\s*Arrival\s*:\n+\s+(\d{2}\/\d{2}\/\d{4} \d{2}:\d{2} [AP]M)\s+(\d{2}\/\d{2}\/\d{4} \d{2}:\d{2} [AP]M)\n+\s*(.+)\s*-\s*\n*\s*Adult\n+\s*(\S+)\n+\s*MYR ([0-9.]+).*\n+\s*(K\w{12,14})\s*\*\*\s*(T\w{12,14})/);

    var extrDataRight = textRight.match(/(.*)\n+(.*)\n+\s*Coach.*Seat\n+(.*)\/(.*)/);
    var hasSeatingData = Boolean(extrDataRight);
    if (!extrDataRight) {
        extrDataRight = textRight.match(/Please scan your\npassport at ACG\n+\s*(\S.*)/);
    }

    for (let r of res) {
        // left side
        r.reservationFor.departureStation.name = extrDataLeft[1];
        r.reservationFor.arrivalStation.name = extrDataLeft[2];
        r.reservationFor.departureTime = JsonLd.toDateTime(extrDataLeft[3], "dd/MM/yyyy hh:mm ap", "en");
        r.reservationFor.arrivalTime = JsonLd.toDateTime(extrDataLeft[4], "dd/MM/yyyy hh:mm ap", "en");
        r.underName.name = extrDataLeft[5];
        // r.underName.identifier = extrDataLeft[6]; // TODO: Confirm where to put ID numbers
        r.totalPrice = parseFloat(extrDataLeft[7]);
        r.priceCurrency = 'MYR';
        // There are two numbers: booking, which is common for all, and ticket, unique for each passenger
        r.reservationNumber = extrDataLeft[9];

        // right side
        if (hasSeatingData) {
            r.reservationFor.trainNumber = extrDataRight[1];
            r.reservationFor.trainName = extrDataRight[2];
            r.reservedTicket.ticketedSeat.seatSection = extrDataRight[3];
            r.reservedTicket.ticketedSeat.seatNumber = extrDataRight[4];
        }
        else if (extrDataRight) {
            r.reservationFor.trainName = extrDataRight[1];
        }
    }

    return res;
}

function extractTextTicket(text) {
    let res = JsonLd.newTrainReservation();

    // TODO: multi passenger support?
    // TODO: not supposed to be there for just "adults"
    // This may include other classes such as "senior" or "student"
    var extrData = text.match(/\s+(.+) - (.+)\n\s*Berlepas \/ Departure :\s*Tiba \/ Arrival :\n\n\s+(\d{2}\/\d{2}\/\d{4} \d{2}:\d{2} [AP]M)\s+(\d{2}\/\d{2}\/\d{4} \d{2}:\d{2} [AP]M)\n\n\s+(.+)\n\s+(.+) - Adult\n\s+(\S+)\s+Coach\/Seat\n\s+MYR ([0-9.]+) \/ .+\S\s\s+(.+)\/(.+)\n\s+(K\d{12})\*\*(T\d{12}) \(Single\)\s+Online\s+(.+)/);

    res.reservationFor.departureStation.name = extrData[1];
    res.reservationFor.arrivalStation.name = extrData[2];
    res.reservationFor.departureTime = JsonLd.toDateTime(extrData[3], "dd/MM/yyyy hh:mm ap", "en");
    res.reservationFor.arrivalTime = JsonLd.toDateTime(extrData[4], "dd/MM/yyyy hh:mm ap", "en");
    res.reservationFor.trainName = extrData[5];
    res.underName.name = extrData[6];
    // res.underName.identifier = extrData[7]; // TODO: Confirm where to put ID numbers
    res.totalPrice = parseFloat(extrData[8]);
    res.priceCurrency = 'MYR';
    res.reservedTicket.ticketedSeat.seatSection = extrData[9];
    res.reservedTicket.ticketedSeat.seatNumber = extrData[10];
    // There are two numbers: booking, which is common for all, and ticket, unique for each passenger
    res.reservationNumber = extrData[12];

    return res;
}
