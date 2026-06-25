// SPDX-FileCopyrightText: 2026 Volker Krause <vkrause@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

function extractTicket(pdf) {
    let reservations = [];
    for (const page of pdf.pages) {
        const text = page.text;
        let baseRes = JsonLd.newBoatReservation();
        baseRes.reservationNumber = text.match(/Ticket(?: number:)? *(IT\d+)\n/)[1];
        const trip = text.match(/Route:? (\S.*\S)-(\S.*\S) +Date: (\d\d-\d\d-\d{4}) .* +Time: *(\d\d:\d\d) /);
        baseRes.reservationFor.departureBoatTerminal.name = trip[1];
        baseRes.reservationFor.arrivalBoatTerminal.name = trip[2];
        baseRes.reservationFor.departureTime = JsonLd.toDateTime(trip[3] + trip[4], "dd-MM-yyyyHH:mm", "en");

        let idx = 0;
        while (true) {
            const pas = text.substr(idx).match(/\d\. (\S.*\S)  .*\n/);
            if (!pas)
                break;
            idx += pas.index + pas[0].length;
            let res = JsonLd.clone(baseRes);
            res.underName.name = pas[1];
            reservations.push(res);
        }
    }
    return reservations;
}
