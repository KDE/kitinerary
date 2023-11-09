/*
    SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

function extractPdf(pdf) {
    let res = JsonLd.newBusReservation();
    const text = pdf.pages[0].text;
    const bookingId = text.match(/ID # ([^\d]*)(\d+)/);
    res.reservationNumber = bookingId[1] + bookingId[2];
    res.reservedTicket.ticketToken = 'qrCode:' + bookingId[2];
    res.reservationFor.departureBusStop.name = text.match(/From: (.*?)  /)[1];
    res.reservationFor.arrivalBusStop.name = text.match(/To: (.*?)  /)[1];
    const dt = text.match(/(\d+ \S{3} \d{4}) (\d\d:\d\d).*(\d\d:\d\d)/);
    res.reservationFor.departureTime = JsonLd.toDateTime(dt[1] + dt[2], 'dd MMM yyyyhh:mm', 'en');
    res.reservationFor.arrivalTime = JsonLd.toDateTime(dt[1] + dt[3], 'dd MMM yyyyhh:mm', 'en');
    const loc = text.match(/(\d+\.\d+) (\d+\.\d+)\n(.*)\n/);
    res.reservationFor.departureBusStop.geo.latitude = loc[1];
    res.reservationFor.departureBusStop.geo.longitude = loc[2];
    const addr = loc[3].split(',');
    res.reservationFor.departureBusStop.address.addressCountry = addr[addr.length - 1];
    res.reservationFor.departureBusStop.address.addressLocality = addr[addr.length - 2];
    res.reservationFor.departureBusStop.address.streetAddress = addr.slice(0, addr.length -2).join(',');
    return res;
}
