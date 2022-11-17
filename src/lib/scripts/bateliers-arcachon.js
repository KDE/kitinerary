/*
   SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
   SPDX-License-Identifier: LGPL-2.0-or-later
*/

function parsePdf(pdf) {
    const text = pdf.text;
    let idx = 0;
    let reservations = [];
    while (true) {
        const trip = text.substr(idx).match(/Billet n° \d+ \/ \d+\n.*\n *(.*?)  +(.*?)  +(.*?)  +(.*)\n *(\d{2}\/\d{2}\/\d{4})  +(\d{2}\/\d{2}\/\d{4})\n *(\d{2}:\d{2})  +(\d{2}:\d{2})  +(\d{2}:\d{2})  +(\d{2}:\d{2})\n +billet n° (\d+)\n/);
        if (!trip) {
            break;
        }
        let out = JsonLd.newBoatReservation();
        out.reservationFor.departureBoatTerminal.name = trip[1];
        out.reservationFor.arrivalBoatTerminal.name = trip[2];
        out.reservationFor.departureTime = JsonLd.toDateTime(trip[5] + trip[7], 'dd/MM/yyyyhh:mm', 'fr');
        out.reservationFor.arrivalTime = JsonLd.toDateTime(trip[5] + trip[8], 'dd/MM/yyyyhh:mm', 'fr');
        out.reservationNumber = trip[11];
        out.reservedTicket.ticketToken = 'barcode128:' + trip[11];
        reservations.push(out);

        let ret = JsonLd.newBoatReservation();
        ret.reservationFor.departureBoatTerminal.name = trip[3];
        ret.reservationFor.arrivalBoatTerminal.name = trip[4];
        ret.reservationFor.departureTime = JsonLd.toDateTime(trip[6] + trip[9], 'dd/MM/yyyyhh:mm', 'fr');
        ret.reservationFor.arrivalTime = JsonLd.toDateTime(trip[6] + trip[10], 'dd/MM/yyyyhh:mm', 'fr');
        ret.reservationNumber = trip[11];
        ret.reservedTicket.ticketToken = 'barcode128:' + trip[11];
        reservations.push(ret);

        idx += trip.index + trip[0].length;
    }
    return reservations;
}
