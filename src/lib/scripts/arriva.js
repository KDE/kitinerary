// SPDX-FileCopyrightText: ⓒ 2025 Volker Krause <vkrause@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

function extractTicket(pdf, node, barcode) {
    const text = pdf.pages[barcode.location].text;
    let res = JsonLd.newBusReservation();
    const trip = text.match(/(\S.*\S)  +(\d{11}-\d)\n+.*\n *(\S.*\S)  +(\d+)\n+.*\n *(\S.*\S)  +\d.*\n *(\S.*\S)\n+.*\n *(\S.*\S)  +(\S.*?)  +(\S.*)\n+.*\n *(\S.*?\S)  +/);
    res.underName.name = trip[1];
    res.reservedTicket.ticketNumber = trip[2];
    res.reservedTicket.name = trip[3];
    res.reservationNumber = trip[4];
    res.reservationFor.departureBusStop.name = trip[5];
    res.reservationFor.arrivalBusStop.name = trip[6];
    res.reservationFor.provider.name = trip[7];
    res.reservedTicket.ticketedSeat.seatNumber = trip[8];
    res.reservationFor.departurePlatform = trip[9];
    res.reservationFor.busName = trip[10];

    const dt = barcode.content.match(/;(\d{6};\d\d:\d\d)/);
    res.reservationFor.departureTime = JsonLd.toDateTime(dt[1], 'ddMMyy;HH:mm', 'hr');

    res.reservedTicket.ticketToken = 'qrcode:' + barcode.content;
    return res;
}
