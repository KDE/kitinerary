// SPDX-FileCopyrightText: 2024 Volker Krause <vkrause@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

function extractEticket(pdf, node, barcode) {
    // boarding pass
    if (barcode.content.repeatedMandatorySection(0).checkinSequenceNumber!= 0 ||
        barcode.content.repeatedMandatorySection(0).seatNumber != 0) {
        const page = pdf.pages[barcode.location].text;
        const depDt = page.match(/Departure.*\n(\d\d:\d\d)[, ]+(\d{2} \S{3} \d{4})/);
        let res = node.result[0];
        res.reservationFor.departureTime = JsonLd.toDateTime(depDt[2] + ' ' + depDt[1], 'dd MMM yyyy HH:mm', 'en');
        const boarding = page.match(/Gate opens.*\n(\d\d:\d\d).*\nBoarding group.*\n(.*?)  /);
        res.reservationFor.boardingTime = JsonLd.toDateTime(boarding[1], 'HH:mm', 'en');
        res.boardingGroup = boarding[2];
        return res;
    }

    // eticket
    const text = pdf.text;
    let reservations = [];
    let idx = 0;
    while (true) {
        const leg = text.substr(idx).match(/(?:(\S.*\S)  +(\S.*\S)|(\S.*\S) (\S.*\S)) +([A-Z0-9]{2})(\d{1,4}) +. +(\d{1,2}[A-Z][a-z]{2}) +(\d\d:\d\d) +(\d\d:\d\d)/);
        if (!leg)
            break;
        idx += leg.index + leg[0].length;
        let res = JsonLd.newFlightReservation();
        res.reservationNumber = barcode.result[0].reservationNumber;
        res.underName = barcode.result[0].underName;
        res.reservedTicket.ticketNumber = barcode.result[0].reservedTicket.ticketNumber;
        res.reservationFor.departureAirport.name = leg[1] ? leg[1] : leg[3];
        res.reservationFor.arrivalAirport.name = leg[2] ? leg[2] : leg[4];
        res.reservationFor.airline.iataCode = leg[5];
        res.reservationFor.flightNumber = leg[6];
        res.reservationFor.departureTime = JsonLd.toDateTime(leg[7] + leg[8], "dMMMhh:mm", "en");
        res.reservationFor.arrivalTime = JsonLd.toDateTime(leg[7] + leg[9], "dMMMhh:mm", "en");
        reservations.push(res);
    }
    return reservations;
}
