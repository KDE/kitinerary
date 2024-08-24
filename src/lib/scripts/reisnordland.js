// SPDX-FileCopyrightText: 2024 Volker Krause <vkrause@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

function extractPdfTicket(pdf, node, barcode)
{
    let res = JsonLd.newBoatReservation();
    res.reservedTicket.ticketToken = "qrcode:" + barcode.content;
    const text = pdf.pages[barcode.location].text;
    res.reservationNumber = text.match(/Ticket code:.*\n(\S+)\n/)[1];
    const dep = text.match(/from:(.*\S) +(\d{4}-\d{2}-\d{2} \d\d:\d\d:\d\d)\n/);
    res.reservationFor.departureBoatTerminal.name = dep[1];
    res.reservationFor.departureTime = JsonLd.toDateTime(dep[2], "yyyy-MM-dd hh:mm:ss", "no");
    const arr = text.match(/to:(.*\S) +(\d{4}-\d{2}-\d{2} \d\d:\d\d:\d\d)\n/);
    res.reservationFor.arrivalBoatTerminal.name = arr[1];
    res.reservationFor.arrivalTime = JsonLd.toDateTime(arr[2], "yyyy-MM-dd hh:mm:ss", "no");
    return res;
}
