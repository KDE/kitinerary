// SPDX-FileCopyrightText: ⓒ 2025 Volker Krause <vkrause@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

function extractTicket(pdf, node, barcode) {
    const text = pdf.pages[barcode.location].text;
    let res = JsonLd.newBusReservation();
    res.reservationNumber = text.match(/Booking Number (.*?)  /)[1];
    res.reservationFor.provider.name = text.match(/operated by:\n+(.*)/)[1];
    const leg = text.match(/(\d{2}\. \S+\. \d{4}, \d\d:\d\d) *\| *(.*)\n.*\n+ *(\d{2}\. \S+\. \d{4}, \d\d:\d\d) *\| *(.*)\n/);
    res.reservationFor.departureTime = JsonLd.toDateTime(leg[1], "dd. MMM. yyyy, HH:mm", "en");
    res.reservationFor.departureBusStop.name = leg[2];
    res.reservationFor.arrivalTime = JsonLd.toDateTime(leg[3], "dd. MMM. yyyy, HH:mm", "en");
    res.reservationFor.arrivalBusStop.name = leg[4];
    res.reservedTicket.ticketToken = 'qrcode:' + barcode.content;
    return res;
}
