// SPDX-FileCopyrightText: 2024 Volker Krause <vkrause@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

function extractItinerary(pdf, node, barcode) {
    let res = JsonLd.newFlightReservation();
    const code = barcode.content.split('|');
    res.reservationNumber = code[1];
    res.underName.name = code[2];
    res.reservedTicket.ticketToken = 'qrcode:' + barcode.content;

    const text = pdf.pages[barcode.location].text;
    const flight = text.match(/([A-Z0-9]{2})(\d{1,4})  +(\w+ \d{2}, \d{4})  +.*?  +(\d\d:\d\d) - (.*?)  +(\d\d:\d\d) - (.*)\n/);
    res.reservationFor.airline.iataCode = flight[1];
    res.reservationFor.flightNumber = flight[2];
    res.reservationFor.departureTime = JsonLd.toDateTime(flight[3] + flight[4], "MMM dd, yyyyhh:mm", "en");
    res.reservationFor.departureAirport.name = flight[5];
    res.reservationFor.arrivalTime = JsonLd.toDateTime(flight[3] + flight[6], "MMM dd, yyyyhh:mm", "en");
    res.reservationFor.arrivalAirport.name = flight[7];
    return res;
}
