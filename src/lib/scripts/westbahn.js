/*
   SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
   SPDX-License-Identifier: LGPL-2.0-or-later
*/

function parseTicket(pdf, node, triggerNode) {
    const page = pdf.pages[triggerNode.location];
    const text = page.text;
    let res = JsonLd.newTrainReservation();

    res.reservedTicket.ticketToken = 'qrcode:' + triggerNode.content;
    const token = triggerNode.content.match(/\/(\d{3})(\d{4})(\d{4})([A-Z0-9]{3})([A-Z0-9]{3})/);
    res.reservedTicket.ticketNumber = token[1] + '-' + token[2] + '-' + token[3];
    res.reservationNumber = token[4] + '-' + token[5];

    const date = text.match(/Gültig am (\d\d\.\d\d.\d{4})/)[1];
    res.reservationFor.departureDay = JsonLd.toDateTime(date, 'dd.MM.yyyy', 'de');

    const rightCol = page.textInRect(0.5, 0.0, 1.0, 1.0);
    const ticketName = rightCol.match(/TICKET\n(.*)/);
    if (ticketName) { // two column layout
        res.reservedTicket.name = ticketName[1];
        const leg = rightCol.match(/VERBINDUNG\n(.*) . (.*)/);
        res.reservationFor.departureStation.name = leg[1];
        res.reservationFor.arrivalStation.name = leg[2];
        const seat = rightCol.match(/Wagen (\d+) .* Sitz (.*)/);
        if (seat) {
            res.reservedTicket.ticketedSeat.seatSection = seat[1];
            res.reservedTicket.ticketedSeat.seatNumber = seat[2];
        }

        const leftCol = page.textInRect(0.0, 0.0, 0.5, 1.0);
        const train = leftCol.match(/(\d\d:\d\d) +(\d\d:\d\d)\n.*\n.*  +(\S.*?)  +/);
        if (train) {
            res.reservationFor.trainNumber = train[3];
            res.reservationFor.departureTime = JsonLd.toDateTime(date + ' ' + train[1], 'dd.MM.yyyy hh:mm', 'de');
            res.reservationFor.arrivalTime = JsonLd.toDateTime(date + ' ' + train[2], 'dd.MM.yyyy hh:mm', 'de');
        }
    } else { // single column layout
        const leg = text.match(/(\d\d:\d\d).*\n.*(\d\d:\d\d)\n(\S.*\S)  +(\S.*\S)\n/);
        res.reservationFor.departureTime = JsonLd.toDateTime(leg[1], "HH:mm", "de");
        res.reservationFor.arrivalTime = JsonLd.toDateTime(leg[2], "HH:mm", "de");
        res.reservationFor.departureStation.name = leg[3];
        res.reservationFor.arrivalStation.name = leg[4];
        const ticket = text.match(/^ *(\d) *West(\S.*)/);
        res.reservedTicket.name = ticket[2];
        res.reservedTicket.ticketedSeat.seatingType = ticket[1];
        const seat = text.match(/Zug *Wagen \/ Deck *Sitz\n *(\S+)  +(\S+).* (\S+)\n/);
        if (seat) {
            res.reservationFor.trainNumber = seat[1];
            res.reservedTicket.ticketedSeat.seatSection = seat[2];
            res.reservedTicket.ticketedSeat.seatNumber = seat[3];
        }
    }

    ExtractorEngine.extractPrice(rightCol, res);
    return res;
}
