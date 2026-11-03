/*
   SPDX-FileCopyrightText: 2026 Carl Schwan <carl@carlschwan.eu>
   SPDX-License-Identifier: LGPL-2.0-or-later
*/

function parseConfirmation(text) {
    const booking = text.match(/Air China-Bestätigungsnummer:\s*([A-Z0-9]{6}) +([A-Z0-9]{6})\b/);
    if (!booking)
        return [];

    const sections = text.split('Passenger Details / Fluggastinformationen');
    if (sections.length !== 2)
        return [];

    let flights = [];
    const legs = sections[0].split(/\n *\S+ (\d{2} \S+ \d{4})\s*\n/);
    for (let i = 1; i < legs.length; i += 2) {
        const leg = legs[i + 1];
        const times = leg.match(/\d\d:\d\d/g);
        const flight = leg.match(/\b([A-Z0-9]{2})(\d{1,4})\b/);
        const airports = leg.match(/\n *([A-Z]{3}) +([A-Z]{3})\b/);
        if (!times || times.length < 2 || !flight || !airports)
            continue;

        let res = JsonLd.newFlightReservation();
        res.reservationNumber = booking[2];
        res.reservationFor.airline.iataCode = flight[1];
        res.reservationFor.flightNumber = flight[2];
        res.reservationFor.departureAirport.iataCode = airports[1];
        res.reservationFor.arrivalAirport.iataCode = airports[2];
        res.reservationFor.departureTime = JsonLd.toDateTime(legs[i] + ' ' + times[0], 'dd MMM yyyy hh:mm', ['de', 'en']);

        // The arrival day offset is printed above the arrival time, sometimes
        // before the departure time in the PDF's reading order.
        const offset = leg.match(/\+(\d+) Tag/);
        const date = new Date(JsonLd.toDateTime(legs[i] + ' 12:00', 'dd MMM yyyy hh:mm', ['de', 'en']));
        const arrivalDate = new Date(Date.UTC(date.getFullYear(), date.getMonth(), date.getDate() + (offset ? Number(offset[1]) : 0)));
        res.reservationFor.arrivalTime = arrivalDate.toISOString().substr(0, 10) + 'T' + times[1] + ':00';

        const terminals = leg.match(/Terminal ([^\n]*?) \/ Terminal \S+ +Terminal ([^\n]*?) \/ Terminal \S+/);
        if (terminals) {
            res.reservationFor.departureTerminal = terminals[1];
            res.reservationFor.arrivalTerminal = terminals[2];
        }
        const cabin = leg.match(/(Economy|Business|First) Class/);
        if (cabin)
            res.reservedTicket.ticketedSeat = { '@type': 'Seat', seatingType: cabin[1] };
        flights.push(res);
    }

    const passengers = sections[1].split(/\n *(\S[^\n]*?) : - [^\n]+\n/);
    let reservations = [];
    for (let i = 1; i < passengers.length; i += 2) {
        const passenger = passengers[i + 1].split('Price Details/')[0];
        const ticket = passenger.match(/e-Flugticket-Nummern: *(\d{3}-\d{10})/);
        for (const flight of flights) {
            let res = JsonLd.clone(flight);
            res.underName.name = passengers[i];
            if (ticket)
                res.reservedTicket.ticketNumber = ticket[1];
            const seat = passenger.match(new RegExp('\\b' + flight.reservationFor.airline.iataCode + flight.reservationFor.flightNumber + ' - (\\d+[A-Z])\\b'));
            if (seat)
                res.airplaneSeat = seat[1];
            reservations.push(res);
        }
    }
    return reservations;
}
