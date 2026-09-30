// SPDX-FileCopyrightText: 2026 Volker Krause <vkrause@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

function extractPdf(pdf, node, barcode) {
    let res = JsonLd.newTrainReservation();
    res.reservedTicket.ticketToken = 'qrcode:' + barcode.content;
    res.reservedTicket.ticketNumber = barcode.content;

    const text = pdf.pages[barcode.location].text;
    res.underName.name = text.match(/.*\n(\S.*\S)\n/)[1];
    res.reservationNumber = text.match(/Booking number: *([A-Z0-9]+)\n/)[1];
    const leg = text.match(/.* (\d\d \S+ \d{4})\n *(\d\d:\d\d) (\S.*)\n *(\d\d:\d\d) (\S.*)\n/);
    res.reservationFor.departureTime = JsonLd.toDateTime(leg[1] + leg[2], "dd MMMM yyyyHH:mm", "en");
    res.reservationFor.departureStation.name = leg[3];
    res.reservationFor.arrivalTime = JsonLd.toDateTime(leg[1] + leg[4], "dd MMMM yyyyHH:mm", "en");
    res.reservationFor.arrivalStation.name = leg[5];
    const seat = text.match(/Train (.*) +Car (.*) +Seat (.*?)(?:\n|  )/);
    res.reservationFor.trainNumber = seat[1];
    res.reservedTicket.ticketedSeat.seatSection = seat[2];
    res.reservedTicket.ticketedSeat.seatNumber = seat[3];
    console.log(seat);
    return res;
}
