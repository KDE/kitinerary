/*
   SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
   SPDX-License-Identifier: LGPL-2.0-or-later
*/

function parseAirport(input) {
    let l = [];
    for (line of input.split('\n')) {
        l.push(line.replace(/^(?:.*?)?  +/, ''));
    }
    return l.join(' ').match(/(\S{2,}.*\S) \(([A-Z]{3})\), (.*)/);
}

function parseETicket(pdf) {
    const pnr = pdf.pages[0].text.match(/Booking Reference +([A-Z0-9]{6})\n/)[1];
    const leftCol = pdf.pages[0].textInRect(0.0, 0.0, 0.44, 1.0);
    let reservations = [];
    for(let idx = 0;;) {
        // select flight number until second date/time
        const row = leftCol.substr(idx).match(/[A-Z0-9]{2} *\d{1,4}  [\s\S]*?\S+, \d+ \S+ \d{4} \d\d:\d\d\n[\s\S]*?\S+, \d+ \S+ \d{4} \d\d:\d\d(?: \+.*)?\n/);
        if (!row)
            break;
        const flight = row[0].match(/([A-Z0-9]{2}) *(\d{1,4}).*\n(.*\S)  +/);
        const depTime = row[0].match(/\S+, (\d+ \S+ \d{4} \d\d:\d\d)\n/);
        const arrTime = row[0].substr(depTime.index + depTime[0].length).match(/\S+, (\d+ \S+ \d{4} \d\d:\d\d)(?: \+.*)?\n/);
        const departure = parseAirport(row[0].substr(0, depTime.index));
        const arrival = parseAirport(row[0].substr(depTime.index + depTime[0].length, arrTime.index));
        idx += row.index + row[0].length;
        let res = JsonLd.newFlightReservation();
        res.reservationFor.airline.iataCode = flight[1];
        res.reservationFor.flightNumber = flight[2];
        res.reservationFor.departureAirport.iataCode = departure[2];
        res.reservationFor.departureAirport.name = departure[3];
        res.reservationFor.airline.name = flight[3];
        res.reservationFor.departureTime = JsonLd.toDateTime(depTime[1], 'd MMM yyyy hh:mm', 'en');
        res.reservationFor.arrivalAirport.iataCode = arrival[2];
        res.reservationFor.arrivalAirport.name = arrival[3];
        res.reservationFor.arrivalTime = JsonLd.toDateTime(arrTime[1], 'd MMM yyyy hh:mm', 'en');
        res.reservationNumber = pnr;
        reservations.push(res);
    }

    return reservations;
}
