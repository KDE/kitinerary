/*
   SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
   SPDX-License-Identifier: LGPL-2.0-or-later
*/

function parseTicket(pdf, node, triggerNode) {
    const page = pdf.pages[triggerNode.location];
    const leftCol = page.textInRect(0.0, 0.0, 0.5, 1.0);
    const rightCol = page.textInRect(0.5, 0.0, 1.0, 1.0);
    let res = JsonLd.newTrainReservation();
    res.reservedTicket.name = rightCol.match(/TICKET\n(.*)/)[1];
    res.reservedTicket.ticketToken = 'qrcode:' + triggerNode.content;
    res.reservedTicket.ticketNumber = rightCol.match(/TICKET NUMMER\n(.*)/)[1];
    res.reservationNumber = rightCol.match(/TICKET CODE\n(.*)/)[1];
    const leg = rightCol.match(/VERBINDUNG\n(.*) . (.*)/);
    res.reservationFor.departureStation.name = leg[1];
    res.reservationFor.arrivalStation.name = leg[2];
    const date = rightCol.match(/GÜLTIGKEIT\n.*(\d\d\.\d\d.\d{4})/)[1];
    res.reservationFor.departureDay = JsonLd.toDateTime(date, 'dd.MM.yyyy', 'de');
    const seat = rightCol.match(/Wagen (\d+) .* Sitz (.*)/);
    if (seat) {
        res.reservedTicket.ticketedSeat.seatSection = seat[1];
        res.reservedTicket.ticketedSeat.seatNumber = seat[2];
    }
    const train = leftCol.match(/(\d\d:\d\d) +(\d\d:\d\d)\n.*\n.*  +(\S.*?)  +/);
    if (train) {
        res.reservationFor.trainNumber = train[3];
        res.reservationFor.departureTime = JsonLd.toDateTime(date + ' ' + train[1], 'dd.MM.yyyy hh:mm', 'de');
        res.reservationFor.arrivalTime = JsonLd.toDateTime(date + ' ' + train[2], 'dd.MM.yyyy hh:mm', 'de');
    }
    return res;
}
