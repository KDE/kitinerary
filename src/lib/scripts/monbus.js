// SPDX-FileCopyrightText: 2026 Volker Krause <vkrause@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

function extractPdf(pdf, node, barcode) {
    const text = pdf.pages[barcode.location].text;
    let res = JsonLd.newBusReservation();
    res.reservedTicket.ticketToken = 'qrcode:' + barcode.content;
    const dep = text.match(/salida\n+\s+(\S.*\S)  +(\d\d:\d\d)/);
    res.reservationFor.departureTime = JsonLd.toDateTime(dep[1] + dep[2], "dd/MM/yyyyHH:mm", "es");
    res.reservationFor.departureBusStop.name = text.match(/Origen\n+\s+(\S.*)\n/)[1];
    const arr = text.match(/llegada\n+\s+(\S.*\S)  +(\d\d:\d\d)/);
    res.reservationFor.arrivalTime = JsonLd.toDateTime(arr[1] + arr[2], "dd/MM/yyyyHH:mm", "es");
    res.reservationFor.arrivalBusStop.name = text.match(/Destino\n+\s+(\S.*)\n/)[1];
    res.reservationFor.provider.name = text.match(/Empresa comercializadora\n+\s+(\S.*)\n/)[1];
    res.reservedTicket.name = text.match(/Tarifa\n+\s+(\S.*)\n/)[1];
    res.reservedTicket.ticketNumber = text.match(/Cod. billete.*\n+\s+(\S+) /)[1];
    res.reservationNumber = text.match(/Cod. Reserva.*\n+\s+(\S+) /)[1];
    res.underName.name = text.match(/Nombre\n+\s+(\S.*)\n/)[1];
    return res;
}
