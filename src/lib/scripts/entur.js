// SPDX-FileCopyrightText: 2024 Volker Krause <vkrause@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

function extractPdf(pdf, node, barcode) {
    const text = pdf.pages[barcode.location].text;

    let res = JsonLd.newTrainReservation();
    res.reservationNumber = text.match(/(?:Reference|Referanse): ([A-Z0-9]+)/)[1];
    const ticketName = text.match(/(?:Ticket|Billett): *(\S.+)\n/);
    res.reservedTicket.name = ticketName ? ticketName[1] : undefined;
    res.reservedTicket.ticketToken = "qrCode:" + barcode.content;

    const dt = text.match(/(?:Valid|Gyldig):? *(?:\S.*?\S )?(\w{3} \d{2}, \d{4}|\d{1,2}\. \S+\.? \d{4})/);
    const trip = text.match(/(\d\d:\d\d) +(\S.*?\S)  +(?:(?:Train|Tog) - (.*)|\S.*\n *(\S.*), Tog.*\n *Vogn.*)\n *(\d\d:\d\d) +(\S.*?\S)(?:  |\n)/);
    res.reservationFor.departureTime = JsonLd.toDateTime(dt[1] + trip[1], ["MMM dd, yyyyHH:mm", "dd. MMMM yyyyHH:mm", "d. MMM. yyyyHH:mm"], ["en", "no"]);
    res.reservationFor.departureStation.name = trip[2];
    res.reservationFor.trainNumber = trip[3] ? trip[3] : trip[4];
    res.reservationFor.arrivalTime = JsonLd.toDateTime(dt[1] + trip[5],  ["MMM dd, yyyyHH:mm", "dd. MMMM yyyyHH:mm", "d. MMM. yyyyHH:mm"], ["en", "no"]);
    res.reservationFor.arrivalStation.name = trip[6];

    const seat = text.match(/(?:Car|Vogn) (\S+)(?: -|,) (?:[Ss]pace|[Ss]ete) (\S.+)\n/);
    if (seat) {
        res.reservedTicket.ticketedSeat.seatSection = seat[1];
        res.reservedTicket.ticketedSeat.seatNumber = seat[2];
    }

    return res;
}
