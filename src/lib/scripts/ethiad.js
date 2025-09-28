// SPDX-FileCopyrightText: 2025 Volker Krause <vkrause@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

function extractTicket(pdf, node) {
    let reservations = node.result;
    const text = pdf.text;
    let idx = 0;

    let baseRes = JsonLd.newFlightReservation();
    baseRes.reservationNumber = text.match(/Booking reference: (.*)\n/)[1];
    baseRes.reservedTicket.ticketNumber = text.match(/Ticket number: (.*)\n/)[1];

    while (true) {
        const leg = text.substr(idx).match(/(?:_([A-Z]{3})\n)? *(\d\d:\d\d) +(\d\d:\d\d)\n *([A-Z]{3})? +([A-Z]{3})\n *(\d\d \S{3} \d{4}) +.* +(\d\d \S{3} \d{4})\n *(\S.*)  +(\S.*)\n *(\S.*)  +(\S.*)\n *(\S.*)  +(\S.*)\n.*?([A-Z0-9]{2})(\d{1,4}).*: (\S.*)  /);
        if (!leg)
            break;
        idx += leg.index +leg[0].length;

        let res = JsonLd.clone(baseRes);
        res.reservationFor.departureAirport.iataCode = leg[1] ?? leg[4];
        res.reservationFor.departureAirport.address.addressLocality = leg[8];
        res.reservationFor.departureAirport.name = leg[10];
        res.reservationFor.departureTerminal = leg[12];
        res.reservationFor.arrivalAirport.iataCode = leg[5];
        res.reservationFor.arrivalAirport.address.addressLocality = leg[9];
        res.reservationFor.arrivalAirport.name = leg[11];
        res.reservationFor.arrivalTerminal = leg[13];

        res.reservationFor.departureTime = JsonLd.toDateTime(leg[2] + ' ' + leg[6], 'HH:mm dd MMM yyyy', 'en');
        res.reservationFor.arrivalTime = JsonLd.toDateTime(leg[3] + ' ' + leg[7], 'HH:mm dd MMM yyyy', 'en');

        res.reservationFor.airline.iataCode = leg[14];
        res.reservationFor.airline.name = leg[16];
        res.reservationFor.flightNumber = leg[15];

        reservations.push(res);
    }

    return reservations;
}
