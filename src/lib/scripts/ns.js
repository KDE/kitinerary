/*
    SPDX-FileCopyrightText: 2024 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

function extractPdf(pdf, node, triggerNode) {
    const text = pdf.pages[triggerNode.location].text;
    let reservations = [];
    let idx = 0;
    while (true) {
        const leg = text.substr(idx).match(/(\d{2}\/\d{2}\/\d{4}) (\d{2}:\d{2}) (\S.*\S) +-> +(\d{2}:\d{2}) (\S.*?\S)  +(\S.*?)  +(\S.*?)  +(\S.*?)\n(?: {14}((?:\S| \S)*)(?:  +(.*))?\n)?/);
        if (!leg)
            break;
        idx += leg.index + leg[0].length;

        let res = JsonLd.clone(triggerNode.result[0]);
        res.reservationFor.departureTime = JsonLd.toDateTime(leg[1] + leg[2], "dd/MM/yyyyhh:mm", "en");
        res.reservationFor.departureStation.name = leg[3] + (leg[9] ? leg[9] : "");
        res.reservationFor.arrivalTime = JsonLd.toDateTime(leg[1] + leg[4], "dd/MM/yyyyhh:mm", "en");
        res.reservationFor.arrivalStation.name = leg[5] + (leg[10] ? leg[10] : "");
        res.reservationFor.trainNumber = leg[6];
        res.reservedTicket.ticketedSeat.seatSection = leg[7] !== '*' ? leg[7] : undefined;
        res.reservedTicket.ticketedSeat.seatNumber = leg[8] !== '*' ? leg[8] : undefined;
        reservations.push(res);
    }

    return reservations;
}
