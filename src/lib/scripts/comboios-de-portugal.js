// SPDX-FileCopyrightText: 2025 Volker Krause <vkrause@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

function parseBarcode(barcode) {
    let code = {};
    for (const entry of barcode.split('*')) {
        const e = entry.split(':');
        code[e[0]] = e[1];
    }
    return code;
}

function extractPdfTicket(pdf, node, barcode) {
    const text = pdf.pages[barcode.location].text;

    let baseRes = JsonLd.newTrainReservation();
    baseRes.reservedTicket.ticketToken = 'qrcode:' + barcode.content;
    const code = parseBarcode(barcode.content);
    baseRes.reservationNumber = code['S'];
    baseRes.reservedTicket.ticketNumber = code['H'];
    baseRes.totalPrice = code['O'];
    baseRes.priceCurrency = 'EUR';

    baseRes.underName.name = text.match(/Nome: (.*)  /)[1];

    const date = text.match(/Data: (\d{4}-\d{2}-\d{2})/)[1];
    let reservations = [];
    let idx = 0;
    while (true) {
        const leg = text.substr(idx).match(/(\d\d:\d\d) (.*) >> (\d\d:\d\d) (.*?)  (\S.*) Classe (.*)/);
        if (!leg)
            break;
        idx += leg.index + leg[0].length;

        let res = JsonLd.clone(baseRes);
        res.reservationFor.departureTime = JsonLd.toDateTime(date + ' ' + leg[1], 'yyyy-MM-dd HH:mm', 'pt');
        res.reservationFor.departureStation.name = leg[2];
        res.reservationFor.arrivalTime = JsonLd.toDateTime(date + ' ' + leg[3], 'yyyy-MM-dd HH:mm', 'pt');
        res.reservationFor.arrivalStation.name = leg[4];
        res.reservationFor.trainNumber = leg[5];
        res.reservedTicket.ticketedSeat.seatingType = leg[6];
        reservations.push(res);
    }

    return reservations;
}
