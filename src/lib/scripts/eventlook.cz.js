/*
    SPDX-FileCopyrightText: 2025 David Pilarčík <meow@charliecat.space>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

function extractPDF(pdf, node) {
    const results = []

    for (const page of pdf.pages) {
        const text = page.text;
        const res = JsonLd.newEventReservation()

        // 05.10.2024 20:00
        res.reservationFor.startDate = JsonLd.toDateTime(text.match(/\d+.\d+.\d+ \d+:\d+/gm)[0], 'dd.MM.yyyy HH:mm', 'cz');

        res.reservationFor.name = page.textInRect(0.0, 0.0, 1, 0.15)

        const addr = page.textInRect(0.0, 0.2, 1, 0.3); // TODO: Handle address parsing, unsure how the address is formatted
        res.reservationFor.location.name = addr;

        res.reservedTicket.ticketedSeat.seatingType = page.textInRect(0.0, 0.35, 1, 0.4) // TODO: Sample had no seat, but had ticket type
        res.reservedTicket.ticketToken = 'qrCode:' + text.match(/\d{14}/)[0];
        res.reservationNumber = text.match(/\d{14}/)[0];

        ExtractorEngine.extractPrice(text, res);
        results.push(res);
    }

    return results
}

function extractPass(pass, node) {
    const res = Object.assign(JsonLd.newEventReservation(), node.result[0])

    res.reservationFor.name = pass.field["event_name"].value ?? res.reservationFor.name;
    res.reservationFor.location = { '@type': 'Place', name: pass.field["location"].value };
    res.reservationNumber = res.reservedTicket.ticketToken.match(/\d{14}/)[0]

    return res
}