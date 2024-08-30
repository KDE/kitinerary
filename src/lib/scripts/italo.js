/*
   SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
   SPDX-License-Identifier: LGPL-2.0-or-later
*/

function parseTicket(content, node, triggerNode) {
    const text = content.pages[triggerNode.location].text;
    let res = JsonLd.newTrainReservation();
    res.reservedTicket.ticketToken = 'azteccode:' + triggerNode.content;
    res.reservationNumber = text.match(/  ([A-Z0-9]{6})  /)[1];
    const dt = text.match(/ (\d\d\/\d\d) /)[1];
    const leg = text.match(/Treno: (\d+) (\d\d:\d\d) (.*?) → (\d\d:\d\d) (.*)\n/);
    res.reservationFor.departureStation.name = leg[3];
    res.reservationFor.departureTime = JsonLd.toDateTime(dt + ' ' + leg[2], 'dd/MM hh:mm', 'en');
    res.reservationFor.arrivalStation.name = leg[5];
    res.reservationFor.arrivalTime = JsonLd.toDateTime(dt + ' ' + leg[4], 'dd/MM hh:mm', 'en');
    res.reservationFor.trainNumber = leg[1];

    let idx = 0;
    let reservations = [];
    while (true) {
        let pas = text.substr(idx).match(/ +\d\. (.*)  +(?:Voiture|Carriage) (\d+) (?:Place|Seat) (\d+)\n/);
        if (!pas) {
            break;
        }
        let r = JsonLd.clone(res);
        r.underName.name = pas[1];
        r.reservedTicket.ticketedSeat.seatSection = pas[2];
        r.reservedTicket.ticketedSeat.seatNumber = pas[3];
        reservations.push(r);
        idx += pas.index + pas[0].length;
    }

    return reservations.length > 0 ? reservations : res;
}

function parsePdfTicket(pdf, node, barcode) {
    const text = pdf.pages[barcode.location].text;
    let res = JsonLd.newTrainReservation();
    res.reservedTicket.ticketToken = 'azteccode:' + barcode.content;
    res.reservationNumber = text.match(/  ([A-Z0-9]{6})\n/)[1];
    const leg = text.match(/(\S.*\S)  +(\S.*\S)  +(\d\d \w{3} \d{4})  +(\S.*\S)  +(\S.*\S)\n.*(\d\d:\d\d) +(\d\d:\d\d)/);
    res.reservationFor.departureStation.name = leg[1];
    res.reservationFor.departureTime = JsonLd.toDateTime(leg[3] + ' ' + leg[6], 'dd MMM yyyy hh:mm', 'it');
    res.reservationFor.arrivalStation.name = leg[2];
    res.reservationFor.arrivalTime = JsonLd.toDateTime(leg[3] + ' ' + leg[7], 'dd MMM yyyy hh:mm', 'it');
    res.reservationFor.trainNumber = leg[4];
    res.reservedTicket.ticketedSeat.seatingType = leg[5];
    res.reservedTicket.ticketNumber = text.match(/(\d+)  +CIV/)[1];

    let idx = 0;
    let reservations = [];
    while (true) {
        let pas = text.substr(idx).match(/(\S.*\S)  +(\S.*\S)  +(\d+)  +(\d+)\n/);
        if (!pas) {
            break;
        }
        let r = JsonLd.clone(res);
        r.underName.name = pas[1];
        r.reservedTicket.name = pas[2];
        r.reservedTicket.ticketedSeat.seatSection = pas[3];
        r.reservedTicket.ticketedSeat.seatNumber = pas[4];
        reservations.push(r);
        idx += pas.index + pas[0].length;
    }

    return reservations;
}
