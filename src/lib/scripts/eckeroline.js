// SPDX-FileCopyrightText: 2024 Volker Krause <vkrause@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

function extractPdf(pdf, node, barcode) {
    const page = pdf.pages[barcode.location].text;
    const trip = page.match(/Boarding card\n(.*)\n(\d{2}\.\d{2}\.\d{4} \d{2}\.\d{2}) (.*) - (.*)\n(.*)\nBooking: (.*)\n/);

    let res = JsonLd.newBoatReservation();
    res.reservedTicket.ticketToken = 'qrCode:' + barcode.content;
    // TODO trip[1] is the vessel name
    res.reservationFor.departureTime = JsonLd.toDateTime(trip[2], 'dd.MM.yyyy hh.mm', 'en');
    res.reservationFor.departureBoatTerminal.name = trip[3];
    res.reservationFor.arrivalBoatTerminal.name = trip[4];
    res.underName.name = trip[5];

    const frontPage = pdf.pages[0].text;
    const arr = frontPage.match(/(\d{2}\.\d{2}\.\d{4}) +(\S.*\S) +(\d{2}\.\d{2}) +(\S.*\S) +(\d{2}\.\d{2})/);
    res.reservationFor.arrivalTime = JsonLd.toDateTime(arr[1] + arr[5], 'dd.MM.yyyyhh.mm', 'en');
    return res;
}
