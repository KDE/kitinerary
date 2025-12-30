/*
   SPDX-FileCopyrightText: 2017 Volker Krause <vkrause@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

function main(text) {
    var reservations = new Array();
    var bookingRef = text.match(/Flug - Buchungscode\s+([0-9A-z]{6})/);
    if (!bookingRef)
        return reservations;

    var pos = bookingRef.index + bookingRef[0].length;
    while (true) {
        var departure = text.substr(pos).match(/[A-Z][a-z] (\d{1,2} \w{3} \d{2})\s+(\d{2}:\d{2})\s+(.+?)\n\t+\((.+?)\)\n/);
        if (!departure)
            break;
        var index = departure.index + departure[0].length;

        var res = JsonLd.newFlightReservation();
        res.reservationNumber = bookingRef[1];
        res.reservationFor.departureTime = JsonLd.toDateTime(departure[1] + ' ' + departure[2], "d MMM yy hh:mm", "en");
        res.reservationFor.departureAirport.name = departure[3] + ", " + departure[4];

        var arrival = text.substr(pos + index).match(/[A-Z][a-z] (\d{1,2} \w{3} \d{2})\s+(\d{2}:\d{2})\s+(.+?)\n\s+\((.+?)\)\n/);
        if (!arrival)
            break;
        index += arrival.index + arrival[1].length;

        res.reservationFor.arrivalTime = JsonLd.toDateTime(arrival[1] + ' ' + arrival[2], "d MMM yy hh:mm", "en");
        res.reservationFor.arrivalAirport.name = arrival[3] + ", " + arrival[4];

        var flightNumber = text.substr(pos + index).match(/Flugnummer: ([A-Z]{2}) (\d{2,4})\n/);
        if (!flightNumber)
            break;
        index += flightNumber.index + flightNumber[0].length;
        res.reservationFor.flightNumber = flightNumber[2];
        res.reservationFor.airline.iataCode = flightNumber[1];

        var opBy = text.substr(pos + index).match(/Durchgeführt von: (.*?)\n/);
        if (!opBy)
            break;
        index += opBy.index + opBy[0].length;
        res.reservationFor.airline.name = opBy[1];

        reservations.push(res);
        if (index == 0)
            break;
        pos += index;
    }
    return reservations;
}

function ticketForYourTripEN(content) {
    const reservations = new Array();
    const klmDateFormat = "dddd d MMMM yyyy - hh:mm";

    const commonInfoElement = content.eval("/html/body/table/tr/td/table/tr/td/table[4]/tr")
    const commonInfoTextString = commonInfoElement[0].recursiveContent
    const bookingRef = commonInfoTextString.match(/Booking code:\s+([0-9A-z]{6})\n/)[1]

    const bookings = content.eval("/html/body/table/tr/td/table/tr/td/table/tr/td/table/tr/td[contains(., 'Booking class:')]")

    for (i in bookings) {
        const booking = bookings[i]
        const res = JsonLd.newFlightReservation();
        res.reservationNumber = bookingRef

        const bookingLines = booking.content.split('\n')
        const firstBookingLine = bookingLines[0]
        const lastBookingLine = bookingLines[bookingLines.length - 1];
        const airports = booking.eval("strong")
        const departureAirport = airports[0].content.match(/, ([A-Z]{3})$/)[1]
        const arrivalAirport = airports[1].content.match(/, ([A-Z]{3})$/)[1]
        const flightNumber = booking.eval("table/tr/td")[0].recursiveContent.match(/^[A-Z]{2}\d+/)[0];
        res.reservationFor.arrivalAirport.iataCode = arrivalAirport
        res.reservationFor.departureAirport.iataCode = departureAirport
        res.reservationFor.departureTime = JsonLd.toDateTime(firstBookingLine, klmDateFormat, "en")
        res.reservationFor.arrivalTime = JsonLd.toDateTime(lastBookingLine, klmDateFormat, "en")
        res.reservationFor.flightNumber = flightNumber
        reservations.push(res)
    }

    return reservations;
}

function extractBoardingPass(pdf, node, barcode) {
    const page = pdf.pages[barcode.location];
    let results = [];
    let offset = 0;
    for (let res of barcode.result) {
        const text = page.textInRect(0, 0 + offset, 0.5, 0.5 + offset);
        offset += 0.5
        res.reservationFor.departureTime = JsonLd.toDateTime(text.match(/(\d\d:\d\d) \/ \d\d \S{3}/)[1], 'hh:mm', 'en');
        res.reservationFor.boardingTime = JsonLd.toDateTime(text.match(/Boarding +(\d\d:\d\d)/)[1], 'hh:mm', 'en');
        var zone = text.match(/Gate closed .* (Zone (?:ZONE)?\d)/);
        if (zone)
            res.boardingGroup = zone[1];
        res.reservationFor.arrivalTime = JsonLd.toDateTime(text.match(/Arrival .* +(\d\d:\d\d)/)[1], 'hh:mm', 'en');
        results.push(res);
    }
    return results;
}
