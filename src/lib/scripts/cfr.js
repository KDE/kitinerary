/*
    SPDX-FileCopyrightText: 2024 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

function extractInternationalPdf(pdf, node, barcode) {
    const text = pdf.pages[barcode.location].text;
    let reservations = [];
    let idx = 0;
    while (true) {
        const leg = text.substr(idx).match(/(\d{2}\.\d{2} \d{2}:\d{2}) +(\S.*?\S)  +(\S.*?\S)  +(\d{2}\.\d{2} \d{2}:\d{2}) .*  (\d+) +(\d+)  +(\S.*)\n/);
        if (!leg)
            break;
        idx += leg.index + leg[0].length;

        let res = JsonLd.clone(node.result[0]);
        res.reservationFor.departureTime = JsonLd.toDateTime(leg[1], "dd.MM hh:mm", "ro");
        res.reservationFor.departureStation.name = leg[2];
        res.reservationFor.arrivalStation.name = leg[3];
        res.reservationFor.arrivalTime = JsonLd.toDateTime(leg[4], "dd.MM hh:mm", "ro");
        res.reservationFor.trainNumber = leg[5];
        res.reservedTicket.ticketedSeat.seatSection = leg[6];
        res.reservedTicket.ticketedSeat.seatNumber = leg[7];
        reservations.push(res);
    }
    return reservations;
}
