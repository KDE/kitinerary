/*
   SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
   SPDX-License-Identifier: LGPL-2.0-or-later
*/

function parsePkPass(pass, node) {
    let res = node.result[0];
    res.reservationFor.name = pass.logoText;
    if (!res.reservationFor.name) {
        const eventLine = pass.field["KEY_EVENTLINE"];
        if (eventLine) {
            res.reservationFor.name = eventLine.value.split("\n")[0];
        }
    }
    const loc = pass.field['KEY_LOCATION'].value.split('\n');
    res.reservationFor.location = JsonLd.newObject('Place');
    res.reservationFor.location.name = loc[0];
    res.reservedTicket.ticketedSeat = JsonLd.newObject('Seat');
    res.reservationFor.location.address = JsonLd.newObject('PostalAddress');
    const addr = loc[1].match(/(.*), (.*?)$/);
    res.reservationFor.location.address.streetAddress = addr[1];
    res.reservationFor.location.address.addressLocality = addr[2];
    res.reservedTicket.ticketedSeat.seatSection = pass.field['KEY_AUX_SEAT_2'].value;
    res.reservedTicket.ticketedSeat.seatRow = pass.field['KEY_AUX_SEAT_3'].value;
    res.reservedTicket.ticketedSeat.seatNumber = pass.field['KEY_AUX_SEAT_4'].value;
    if (!res.reservedTicket.ticketedSeat.seatNumber) {
        res.reservedTicket.ticketedSeat.seatNumber = pass.field['KEY_BACK_SEATLINE'].value;
    }
    return res;
}

function parseTicketDirect(pdf, node, triggerNode)
{
    const text = pdf.pages[0].textInRect(0.0, 0.0, 0.875, 0.5);

    let res = JsonLd.newEventReservation();
    res.reservationNumber = text.match(/Ticket Nummer: (\d+-\d+)\n/)[1];
    const dt = text.match(/([A-Z][a-z]+, \d{2}\.\d{2}\.\d{4}).*\n.*(\d{2}:\d{2}) Uhr(?:.*\n.*?(\d{2}:\d{2}) Uhr.*)?/);

    if (dt[3]) {
        res.reservationFor.doorTime = JsonLd.toDateTime(dt[1] + dt[2], 'dddd, dd.MM.yyyyhh:mm', 'de');
        res.reservationFor.startDate = JsonLd.toDateTime(dt[1] + dt[3], 'dddd, dd.MM.yyyyhh:mm', 'de');
    } else {
        res.reservationFor.startDate = JsonLd.toDateTime(dt[1] + dt[2], 'dddd, dd.MM.yyyyhh:mm', 'de');
    }

    res.reservationFor.name = text.substr(dt.index + dt[0].length).match(/(.+\n.*)\n/)[1];
    res.reservedTicket.ticketToken = 'qrcode:' + text.match(/\b(\d{22})\b/)[1];

    const seat = text.match(/Block: (.*) +Reihe: (.*) +Sitz: (.*)/);
    if (seat) {
        res.reservedTicket.ticketedSeat.seatSection = seat[1];
        res.reservedTicket.ticketedSeat.seatRow = seat[2];
        res.reservedTicket.ticketedSeat.seatNumber = seat[3];
    }

    return res;
}
