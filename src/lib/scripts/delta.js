/*
   SPDX-FileCopyrightText: 2026 leo vriska <leo@60228.dev>
   SPDX-License-Identifier: LGPL-2.0-or-later
*/

function main(html) {
    const reservations = [];

    const [confirmationNumber] = html.eval("//b[text()='Confirmation Number']/../../div//span");

    const passengers = html.eval("//strong[text()='Passenger Info']/../../..").map(info => ({
        name: /Name: (.+)/.exec(info.eval(".//td[1]")[0]?.content)[1],
        seats: info.eval(".//tr/td[not(*)][2]").map(x => x.content),
    }));

    // The itinerary table only contains airport names, but the baggage allowance table uses IATA codes.
    const baggageHeaders = html.eval("//strong[text()='Checked Bag Allowance']/../../../tr[count(td[b])=2]");
    const segments = html.eval("//b[text()='DEPART']/../../following-sibling::tr[1]");

    for (let i = 0; i < segments.length; i++) {
        const baggageHeader = baggageHeaders[i];
        const segment = segments[i];

        const [date, airports] = baggageHeader.eval(".//b").map(x => x.content);
        const [departureAirport, arrivalAirport] = airports.split("-");

        const [flight, departure, arrival] = segment.eval("./td");

        const [, airline, flightNumber] = /^(.+?) ([0-9]+)\*?$/m.exec(flight.content);

        const [departureAirportName, departureTime] = departure.content.split(/\s+/g);
        const [arrivalAirportName, arrivalTime] = arrival.content.split(/\s+/g);

        const dateFormat = "ddd dd MMM yyyy hh:mmAP";
        const departureDatetime = JsonLd.toDateTime(date + " " + departureTime, dateFormat, "en");
        const arrivalDatetime = JsonLd.toDateTime(date + " " + arrivalTime, dateFormat, "en");

        for (const {name, seats} of passengers) {
            const res = JsonLd.newFlightReservation();

            res.reservationFor.airline.name = airline;
            if (airline === "DELTA") res.reservationFor.airline.iataCode = "DL";

            res.reservationNumber = confirmationNumber?.content;

            res.reservationFor.flightNumber = flightNumber;

            res.reservationFor.departureTime = departureDatetime;
            res.reservationFor.departureAirport.iataCode = departureAirport;
            res.reservationFor.departureAirport.name = departureAirportName;
            res.reservationFor.arrivalTime = arrivalDatetime;
            res.reservationFor.arrivalAirport.iataCode = arrivalAirport;
            res.reservationFor.arrivalAirport.name = arrivalAirportName;

            res.underName.name = name;
            res.airplaneSeat = seats[i];

            reservations.push(res);
        }
    }

    const [price] = html.eval("//b[text()='METHOD OF PAYMENT']/../../following-sibling::tr[1]/td/b");

    if (price)
        ExtractorEngine.extractPrice(price.recursiveContent, reservations);

    return reservations;
}
