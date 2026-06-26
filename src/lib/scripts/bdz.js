// SPDX-FileCopyrightText: 2026 Volker Krause <vkrause@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

function extractPdfTicket(pdf, node) {
    let reservations = [];
    for (page of pdf.pages) {
        const text = page.text;
        let res = JsonLd.newTrainReservation();
        const hdr = text.match(/\S  +(\S.*)\n *№: (\d+) .*: (.*)\n/);
        res.underName.name = hdr[1];
        res.reservationNumber = hdr[2];
        res.reservedTicket.name = hdr[3];

        const leg = text.match(/(\d\d\.\d\d.\d\d) - (\d\d:\d\d) (.*) - (.*) \((.*)\) +(\d\d\.\d\d\.\d\d) - (\d\d:\d\d) +(\d)(?: +(\d+) +(\d+))?/);

        res.reservationFor.departureTime = JsonLd.toDateTime(leg[1] + leg[2], 'dd.MM.yyHH:mm', 'bg');
        res.reservationFor.departureStation.name = leg[3];
        res.reservationFor.arrivalStation.name = leg[4];
        res.reservationFor.trainNumber = leg[5];
        res.reservationFor.arrivalTime = JsonLd.toDateTime(leg[6] + leg[7], 'dd.MM.yyHH:mm', 'bg');
        res.reservedTicket.ticketedSeat.seatingType = leg[8];
        res.reservedTicket.ticketedSeat.seatSection = leg[9];
        res.reservedTicket.ticketedSeat.seatNumber = leg[10];
        const barcode = node.findChildNodes({ scope: "Descendants", mimeType: "application/octet-stream" })[0];
        res.reservedTicket.ticketToken = 'qrcodebin:' + ByteArray.toBase64(barcode.content);

        const price = text.match(/(\d+\.\d\d) (?:euro|евро)/);
        res.priceCurrency = "EUR";
        res.totalPrice = price[1];

        reservations.push(res);
    }
    return reservations;
}
