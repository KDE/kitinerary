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
    res.reservationFor.departureBusStop.name = text.match(/From: *(\S.*?\S)  /)[1];
    res.reservationFor.arrivalBusStop.name = text.match(/To: *(\S.*?\S)  /)[1];
    const dt = text.match(/(\d+ \S{3} \d{4}) (\d\d:\d\d).*\n?.*Arrival .*(\d\d:\d\d)/);
    res.reservationFor.departureTime = JsonLd.toDateTime(dt[1] + dt[2], 'dd MMM yyyyhh:mm', 'en');
    res.reservationFor.arrivalTime = JsonLd.toDateTime(dt[1] + dt[3], 'dd MMM yyyyhh:mm', 'en');
    const loc = text.match(/(\d+\.\d+) ,?(\d+\.\d+)\n(.*)\n/);
    res.reservationFor.departureBusStop.geo.latitude = loc[1];
    res.reservationFor.departureBusStop.geo.longitude = loc[2];
    const addr = loc[3].split(',');
    res.reservationFor.departureBusStop.address.addressCountry = addr[addr.length - 1];
    res.reservationFor.departureBusStop.address.addressLocality = addr[addr.length - 2];
    res.reservationFor.departureBusStop.address.streetAddress = addr.slice(0, addr.length -2).join(',');
    const currency = text.match(/Passenger.*\s+([A-Z]{3})/);
    if (currency) {
        res.priceCurrency = currency[1];
        res.totalPrice = text.match(/Total\s+([\d,]+)\n/)[1].replace(',', '');
    }
    return res;
}

function fixSchemaOrg(html, node) {
    let results = node.result;
    for (res of results) {
        // doesn't match what's in the PDF
        res.reservationNumber = undefined;
        if (res.reservationFor.departureTime == res.reservationFor.arrivalTime)
            res.reservationFor.arrivalTime = undefined;
        // contains server rather than location timezone
        res.reservationFor.departureTime = res.reservationFor.departureTime.substr(0, 19);
    }
    return results;
}
