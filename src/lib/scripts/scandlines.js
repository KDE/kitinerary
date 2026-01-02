// SPDX-FileCopyrightText: 2024 Volker Krause <vkrause@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

function extractPdf(pdf, node, barcode) {
    const text = pdf.text;
    let idx = 0;
    let reservations = [];
    while (true) {
        const trip = text.substr(idx).match(/:  +(\S.*\S)  +.*:  +(\d{2}.\d{2}.\d{4} \d\d:\d\d)\n.*:  +(\S.*\S)  +.*:  +(\d{2}.\d{2}.\d{4} \d\d:\d\d)\n/);
        if (!trip)
            break;
        idx += trip.index + trip[0].length;

        let res = JsonLd.newBoatReservation();
        res.reservationFor.departureBoatTerminal.name = trip[1];
        res.reservationFor.departureTime = JsonLd.toDateTime(trip[2], "dd.MM.yyyy hh:mm", "dk");
        res.reservationFor.arrivalBoatTerminal.name = trip[3];
        res.reservationFor.arrivalTime = JsonLd.toDateTime(trip[4], "dd.MM.yyyy hh:mm", "dk");
        res.reservedTicket.ticketToken = 'qrcode:' + barcode.content;
        res.reservationNumber = barcode.content;
        reservations.push(res);
    }
    return reservations;
}
