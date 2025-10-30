// SPDX-FileCopyrightText: 2025 Volker Krause <vkrause@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

function extractPdfTicket(pdf, node, barcode) {
    const text = pdf.pages[barcode.location].text;
    let reservations = [];
    let idx = 0;

    let baseRes = JsonLd.newTrainReservation();
    baseRes.reservedTicket.ticketToken = 'qrcode:' + barcode.content;
    baseRes.reservationNumber = text.match(/Ticket Id: (\d+)/)[1];

    while (true) {
        const leg = text.substr(idx).match(/(\d{2} \S{3}\. \d{4})  +(\d\d:\d\d)  +(\S.*\S)  +(\S.*\S)  +(\d\d:\d\d)  +(\d{2} \S{3}\. \d{4})\n.*\n *(\d+,\d{2}) lei\n *(\S+ \d+)  +(\d+)  +(\d+)  +(\S.*)\n/);
        console.log(leg);
        if (!leg)
            break;
        idx += leg.index + leg[0].length;

        let res = JsonLd.clone(baseRes);
        res.reservationFor.departureTime = JsonLd.toDateTime(leg[1] + ' ' + leg[2], 'dd MMM. yyyy HH:mm', ['ro', 'en']);
        res.reservationFor.departureStation.name = leg[3];
        res.reservationFor.arrivalStation.name = leg[4];
        res.reservationFor.arrivalTime = JsonLd.toDateTime(leg[6] + ' ' + leg[5], 'dd MMM. yyyy HH:mm', ['ro', 'en']);
        res.totalPrice = leg[7].replace(',', '.');
        res.priceCurrency = 'RON';
        res.reservationFor.trainNumber = leg[8];
        res.reservedTicket.ticketedSeat.seatSection = leg[9];
        res.reservedTicket.ticketedSeat.seatNumber = leg[10];
        res.reservedTicket.ticketedSeat.seatingType = leg[11];
        reservations.push(res);
    }

    return reservations;
}
