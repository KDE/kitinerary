// SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

function extractReservation(pdf, node, barcode) {
    let reservations = [];
    for (let i = barcode.location; i < barcode.location + 2; ++i) {
        const page = pdf.pages[i].text;
        let idx = 0;
        while (true) {
            const leg = page.substr(idx).match(/(\d\d\.\S{3}\.) +(\d\d:\d\d) +(\S.*\S)  +(\S.*\S)  +(\d\d\.\S{3}\.)  +(\d\d:\d\d)\n.*\n(\S.*?\S)  +(\d+)  +(\S.*?\S)  +\d  +(\S.*?\S)  +(\S.*\S)\n(\d+)?/)
            if (!leg)
                break;
            idx += leg.index + leg[0].length;

            let res = JsonLd.newTrainReservation();
            res.reservationFor.departureTime = JsonLd.toDateTime(leg[1] + leg[2], "dd.MMM.HH:mm", "da");
            res.reservationFor.departureStation.name = leg[3];
            res.reservationFor.arrivalStation.name = leg[4];
            res.reservationFor.arrivalTime = JsonLd.toDateTime(leg[5] + leg[6], "dd.MMM.HH:mm", "da");
            res.reservationFor.trainNumber = leg[7] + " " + (leg[12] ? leg[12] : "");
            res.reservedTicket.ticketedSeat.seatSection = leg[8];
            res.reservedTicket.ticketedSeat.seatNumber = leg[9];
            res.reservedTicket.name = leg[10];
            reservations.push(res);
        }

    }
    return reservations;
}

function extractItinerary(pdf, node, barcode) {
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
        if (leg[5].match(/togbus/i)) {
            res = JsonLd.trainToBusReservation(res);
        }
        reservations.push(res);
    }

    return reservations;
}
