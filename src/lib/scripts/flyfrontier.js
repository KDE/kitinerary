/*
   SPDX-FileCopyrightText: 2026 leo vriska <leo@60228.dev>
   SPDX-License-Identifier: LGPL-2.0-or-later
*/

function main(html) {
    const reservations = [];

    const [bookingCode] = html.eval("//div[@class='confirmationcode-fg-pill-copy']");

    const [passengerDetails] = html.eval("//td[normalize-space(text())='Passenger Details']/ancestor::table[@align='center'][1]/following-sibling::table[1]");

    // FIXME: how does this work for multiple passengers?
    const name = passengerDetails?.eval(".//td[not(*)]")?.[0];

    // FIXME: how does this work for one-way and multi-city?
    const outboundSeats = passengerDetails?.eval(".//tr[td[normalize-space(text())='Outbound Seats']]/following-sibling::tr/td")?.[0];
    const returnSeats = passengerDetails?.eval(".//tr[td[normalize-space(text())='Return Seats']]/following-sibling::tr/td")?.[0];
    const seats = [outboundSeats, returnSeats];

    const dates = html.eval("//td[@class='itinerary-cl-sectionbar']/descendant::td[not(*)][last()]").map(x => x.content.split(': ').pop());
    const segments = html.eval("//div[normalize-space(text())='Departs']/ancestor::table[2]");

    for (let i = 0; i < segments.length; i++) {
        const date = dates[i];
        const seat = seats[i];
        const segment = segments[i];

        const res = JsonLd.newFlightReservation();

        // Frontier does not have any interline agreements
        res.reservationFor.airline.iataCode = "F9";
        res.reservationFor.airline.name = "Frontier Airlines";

        res.reservationNumber = bookingCode?.content;

        const [flightNumber] = segment.eval(".//*[(self::td or self::div) and normalize-space(text()[1])='Flight' and starts-with(normalize-space(text()[2]), '#')]");

        if (flightNumber)
            res.reservationFor.flightNumber = flightNumber.recursiveContent.split("#")[1];

        const [departure, departureAirportName] = segment.eval(".//div[preceding-sibling::div[normalize-space(text())='Departs']]");

        if (departure) {
            const [departureAirport, departureTime] = departure.recursiveContent.split("\n");
            res.reservationFor.departureTime = JsonLd.toDateTime(date + " " + departureTime, "MMMM d, yyyy h:mm ap", "en");
            res.reservationFor.departureAirport.iataCode = departureAirport;
            res.reservationFor.departureAirport.name = departureAirportName?.content;
        }

        const [arrival, arrivalAirportName] = segment.eval(".//div[preceding-sibling::div[normalize-space(text())='Arrives']]");

        if (arrival) {
            const [arrivalAirport, arrivalTime] = arrival.recursiveContent.split("\n");
            res.reservationFor.arrivalTime = JsonLd.toDateTime(date + " " + arrivalTime, "MMMM d, yyyy h:mm ap", "en");
            res.reservationFor.arrivalAirport.iataCode = arrivalAirport;
            res.reservationFor.arrivalAirport.name = arrivalAirportName?.content;
        }

        if (name)
            res.underName.name = name.recursiveContent.replace(/^(.+ .+) \1$/, "$1");

        if (seat)
            res.airplaneSeat = seat.content.split(' ').pop();

        reservations.push(res);
    }

    const [price] = html.eval("//td[normalize-space(text())='Payment Amount']/following-sibling::td[@class='receipt-al-amount']");

    if (price)
        ExtractorEngine.extractPrice(price.recursiveContent + ' USD', reservations);

    return reservations;
}
