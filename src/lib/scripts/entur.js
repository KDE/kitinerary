// SPDX-FileCopyrightText: 2024 Volker Krause <vkrause@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

function extractPdf(pdf, node) {
    const text = pdf.pages[0].text; // can this have multiple pages?

    let res = JsonLd.newTrainReservation();
    res.reservationNumber = text.match(/Reference: ([A-Z0-9]+)/)[1];
    res.reservedTicket.name = text.match(/Ticket: *(\S.+)\n/)[1];
    res.reservedTicket.ticketToken = "qrCode:" + node.findChildNodes({ scope: "Descendants", mimeType: "text/plain", match: "^[A-Za-z0-9+/=]+$" })[0].content;

    const dt = text.match(/Valid: *\S.*?\S (\w{3} \d{2}, \d{4})\n/);
    const trip = text.match(/(\d\d:\d\d) +(\S.*?\S)  +Train - (.*)\n *(\d\d:\d\d) +(\S.*?\S)  /);
    console.log(trip);
    res.reservationFor.departureTime = JsonLd.toDateTime(dt[1] + trip[1], "MMM dd, yyyyhh:mm", "en");
    res.reservationFor.departureStation.name = trip[2];
    res.reservationFor.trainNumber = trip[3];
    res.reservationFor.arrivalTime = JsonLd.toDateTime(dt[1] + trip[4], "MMM dd, yyyyhh:mm", "en");
    res.reservationFor.arrivalStation.name = trip[5];

    const seat = text.match(/Car (\S+) - space (\S+)/);
    if (seat) {
        res.reservedTicket.ticketedSeat.seatSection = seat[1];
        res.reservedTicket.ticketedSeat.seatNumber = seat[2];
    }

    return res;
}
