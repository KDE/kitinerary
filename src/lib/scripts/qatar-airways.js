/*
   SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
   SPDX-License-Identifier: LGPL-2.0-or-later
*/

function parseETicket(pdf) {
    const pnr = pdf.pages[0].text.match(/Booking Reference +([A-Z0-9]{6})\n/)[1];
    const leftCol = pdf.pages[0].textInRect(0.0, 0.0, 0.44, 1.0);
    let reservations = [];
    for(let idx = 0;;) {
        const leg = leftCol.substr(idx).match(/([A-Z0-9]{2})(\d{1,4})  +... \(([A-Z]{3})\), (.*)\n(.*?)  +\S*, (.*)\n.*\n *... \(([A-Z]{3})\), (.*)\n.*, (.*)\n/);
        console.log(leg);
        if (!leg)
            break;
        idx += leg.index + leg[0].length;
        let res = JsonLd.newFlightReservation();
        res.reservationFor.airline.iataCode = leg[1];
        res.reservationFor.flightNumber = leg[2];
        res.reservationFor.departureAirport.iataCode = leg[3];
        res.reservationFor.departureAirport.name = leg[4];
        res.reservationFor.airline.name = leg[5];
        res.reservationFor.departureTime = JsonLd.toDateTime(leg[6], 'd MMM yyyy hh:mm', 'en');
        res.reservationFor.arrivalAirport.iataCode = leg[7];
        res.reservationFor.arrivalAirport.name = leg[8];
        res.reservationFor.arrivalTime = JsonLd.toDateTime(leg[9], 'd MMM yyyy hh:mm', 'en');
        res.reservationNumber = pnr;
        reservations.push(res);
    }

    return reservations;
}
