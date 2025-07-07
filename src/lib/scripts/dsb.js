// SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

function extractPdf(pdf, node, barcode) {
    let page = pdf.pages[barcode.location + 1].text;
    if (!page.match(/^\s*Din Rejseplan/))
        page = pdf.pages[barcode.location + 2].text;
    const baseRes = barcode.result[0];

    const isValidBaseRes = !baseRes.reservationFor.departureStation.name.match(/^\d+$/);
    let reservations = isValidBaseRes ? [baseRes] : [];
    let idx = page.indexOf("Detaljer");
    while (true) {
        const leg = page.substr(idx).match(/(\S.*\S) +Afg: +(\d\d:\d\d)\n *(\S.*\S) +Ank: +(\d\d:\d\d) +(\S.*) til/);
        if (!leg)
            break;
        idx += leg.index + leg[0].length;
        if (leg[5].match(/Gåtur|Walk/))
            continue;
        let res = isValidBaseRes ? JsonLd.newTrainReservation() : JsonLd.clone(baseRes);
        res.reservationFor.departureStation.name = leg[1];
        res.reservationFor.departureTime = JsonLd.toDateTime(baseRes.reservationFor.departureDay + leg[2], 'yyyy-MM-ddhh:mm', 'dk');
        res.reservationFor.arrivalStation.name = leg[3];
        res.reservationFor.arrivalTime =
        JsonLd.toDateTime(baseRes.reservationFor.departureDay + leg[4], 'yyyy-MM-ddhh:mm', 'dk');
        res.reservationFor.trainNumber = leg[5];
        reservations.push(res);
    }

    return reservations;
}
