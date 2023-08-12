/*
   SPDX-FileCopyrightText: 2017 Volker Krause <vkrause@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

function main(text) {
    let reservations = [];
    let traveler = [];

    let bookingRef = text.match(/(?:Airline booking reference|Flygbolagets bokningsreferens): ([A-Z0-9]{6})/);
    if (!bookingRef)
        bookingRef = text.match(/(?:[Bb]ooking reference|[Bb]okningsreferens)\s*:\s*([A-Z0-9]{6})/);
    if (!bookingRef)
        return;

    let pos = 0;
    while (true) {
        const flightLine = text.substr(pos).match(/(?:Flight|Flyg):\s+([A-Z0-9]{2}) *([0-9]{1,4}), (.+)\n/);
        if (!flightLine)
            break;
        let index = flightLine.index + flightLine[0].length;

        let res = JsonLd.newFlightReservation();
        res.reservationNumber = bookingRef[1];
        res.reservationFor.flightNumber = flightLine[2];
        res.reservationFor.airline.iataCode = flightLine[1];
        res.reservationFor.airline.name = flightLine[3];

        const depDate = text.substr(pos + index).match(/(?:Date|Datum): *\S{3} (.*)\n/);
        if (depDate)
            index += depDate.index + depDate[0].length;
        const depLine = text.substr(pos + index).match(/(?:Departure|Avgång): *([0-9]+:[0-9]+) *(.*) *\(([A-Z]{3})\)/);
        if (!depLine)
            break;
        index += depLine.index + depLine[0].length;
        res.reservationFor.departureTime = JsonLd.toDateTime(depDate[1] + " " + depLine[1], "d MMM yyyy hh:mm", ['en', 'sv']);
        res.reservationFor.departureAirport.name = depLine[2];
        res.reservationFor.departureAirport.iataCode = depLine[3];

        let arrDate = text.substr(pos + index).match(/(?:Date|Datum): *[A-Z][a-z]{2} (.*)\n/);
        const arrLine = text.substr(pos + index).match(/(?:Arrival|Ankomst): *([0-9]+:[0-9]+) *(.*) *\(([A-Z]{3})\)/);
        if (!arrLine)
            break;
        if (!arrDate || arrDate.index > arrLine.index) // arrival date is sometimes optional
            arrDate = depDate;
        index += arrLine.index + arrLine[0].length;
        res.reservationFor.arrivalTime = JsonLd.toDateTime(arrDate[1] + " " + arrLine[1], "d MMM yyyy hh:mm", ['en', 'sv']);
        res.reservationFor.arrivalAirport.name = arrLine[2];
        res.reservationFor.arrivalAirport.iataCode = arrLine[3];

        reservations.push(res);
        if (index == 0)
            break;
        pos += index;
    }

    if (reservations.length > 0) {
        pos = 0;
        while(true) {
            var name = text.substr(pos).match(/(?:Traveller|Resenär):\s+(.+)\n/);
            if (!name)
                break;
            pos += name.index + name[0].length;
            traveler.push(name[1]);
        }
    }

    // merge traveler and reservation data
    if (traveler.length == 0)
        return reservations;
    var result = new Array();
    for (var i = 0; i < reservations.length; ++i) {
        for (var j = 0; j < traveler.length; ++j) {
            // urgh, but apparently that's the way to clone objects in JS
            // without that we only have one reference that then will end up all with the same person name
            var r = JsonLd.clone(reservations[i]);
            r.underName.name = traveler[j];
            result.push(r);
        }
    }

    const price = text.match(/(?:Total price|Totalpris):\s*(.*)/);
    if (price)
        ExtractorEngine.extractPrice(price[1], result);
    return result;
}
