// SPDX-FileCopyrightText: 2024 Volker Krause <vkrause@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

function extractPdfTicket(pdf, node, barcode) {
    const text = pdf.pages[barcode.location].text;

    let reservations = [];
    let idx = 0;
    while (true) {
        const leg = text.substr(idx).match(/(\d\d\.\d\d \d\d:\d\d) +(\S.*\S) +-> +(\S.*\S) +(\d\d\.\d\d \d\d:\d\d) +(\d)\n.*\n *(\S.*?\S)  +(\d+)  +(\s.*)\n/);
        if (!leg)
            break;
        idx += leg.index + leg[0].length;

        let res = JsonLd.newTrainReservation();
        res.reservedTicket.ticketToken ="azteccode:" + barcode.content;
        res.reservationFor.departureTime = JsonLd.toDateTime(leg[1], "dd.MM hh:mm", "pl");
        res.reservationFor.departureStation.name = leg[2];
        res.reservationFor.arrivalStation.name = leg[3];
        res.reservationFor.arrivalTime = JsonLd.toDateTime(leg[4], "dd.MM hh:mm", "pl");
        res.reservedTicket.ticketedSeat.seatingType = leg[5];
        res.reservationFor.trainNumber = leg[6];
        res.reservedTicket.ticketedSeat.seatSection = leg[7];
        res.reservedTicket.ticketedSeat.seatNumber = leg[8];
        reservations.push(res);
    }
    return reservations;
}
