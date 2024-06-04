/*
   SPDX-FileCopyrightText: 2017 Volker Krause <vkrause@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

function main(text) {
    var reservations = new Array();
    var bookingRef = text.match(/BOOKING REF: ([A-Z0-9]{6})/);

    var pos = 0;
    while (true) {
        var flightHeader = text.substr(pos).match(/FLIGHT *([A-Z0-9]{2}) ([0-9]{3,4}) - ([A-Za-z0-9 ]*?)  .*([0-9]{4})\n/);
        if (!flightHeader)
            break;
        var idx = flightHeader.index + flightHeader[0].length;

        var res = JsonLd.newFlightReservation();
        res.reservationNumber = bookingRef[1];
        res.reservationFor.airline.iataCode = flightHeader[1];
        res.reservationFor.airline.name = flightHeader[3];
        res.reservationFor.flightNumber = flightHeader[2];

        // TODO support line continuation for DEPARTURE/ARRIVAL
        var depLine = text.substr(pos + idx).match(/DEPARTURE: +(.*?) [ -] *([0-9]{2} [A-Z]{3}) ([0-9]{2}:[0-9]{2})/);
        if (!depLine)
            break;
        idx = depLine.index + depLine[0].length;
        res.reservationFor.departureAirport.name = depLine[1];
        res.reservationFor.departureTime = JsonLd.toDateTime(depLine[2] + ' ' + flightHeader[4] + ' ' + depLine[3], "dd MMM yyyy hh:mm", "en");

        var arrLine = text.substr(pos + idx).match(/ARRIVAL: +(.*?) [ -] *([0-9]{2} [A-Z]{3}) ([0-9]{2}:[0-9]{2})/);
        if (!arrLine)
            break;
        idx = arrLine.index + arrLine[0].length;
        res.reservationFor.arrivalAirport.name = arrLine[1];
        res.reservationFor.arrivalTime = JsonLd.toDateTime(arrLine[2] + ' ' + flightHeader[4] + ' ' + arrLine[3], "dd MMM yyyy hh:mm", "en");

        reservations.push(res);
        if (idx == 0)
            break;
        pos += idx;
    }

    return reservations;
}

function parseEvent(event)
{
    var res = JsonLd.newFlightReservation();

    // force UTC, otherwise we lose the timezone due to JS converting to the local TZ
    res.reservationFor.departureTime = event.dtStart.toJSON();
    res.reservationFor.arrivalTime = event.dtEnd.toJSON();
    res.reservationNumber = event.uid.substr(0, 6);

    var flight = event.description.match(/Flight: ?(.*) - (\S{2}) (\S{1,4})\n/);
    res.reservationFor.airline.name = flight[1];
    res.reservationFor.airline.iataCode = flight[2];
    res.reservationFor.flightNumber = flight[3];

    var from = event.description.match(/From: ?(.*)\n/);
    res.reservationFor.departureAirport.name = from[1];

    var to = event.description.match(/To: ?(.*)\n/);
    res.reservationFor.arrivalAirport.name = to[1];

    return res;
}

function parseCytricEvent(ev)
{
    const category = ev.categories[0];
    if (category.match(/Flight|Flug/)) {
        let reservations = [];
        let idx = 0;
        while (true) {
            let res = JsonLd.newFlightReservation();
            const flight = ev.description.substr(idx).match(/([A-Z0-9]{2}) (.*?) (\d{1,4}) .*\n.* - (.*) \(([A-Z]{3})\)(?:, Terminal (.*?),)? .*?, (\d+ \S+ \d{4}) .*? (\d\d:\d\d)\n.*? - (.*) \(([A-Z]{3})\)(?:, Terminal (.*?),)? .*?, (\d+ \S+ \d{4}) .* (\d\d:\d\d)\n[\s\S]*?: ([A-Z0-9]{6})\n/);
            if (!flight)
                break;
            idx += flight.index + flight[0].length;
            res.reservationFor.airline.iataCode = flight[1];
            res.reservationFor.airline.name = flight[2];
            res.reservationFor.flightNumber = flight[3];

            res.reservationFor.departureAirport.name = flight[4];
            res.reservationFor.departureAirport.iataCode = flight[5];
            res.reservationFor.departureTerminal = flight[6];
            res.reservationFor.departureTime = JsonLd.toDateTime(flight[7] + flight[8], "dd MMMM yyyyhh:mm", ["en", "de"]);

            res.reservationFor.arrivalAirport.name = flight[9];
            res.reservationFor.arrivalAirport.iataCode = flight[10];
            res.reservationFor.arrivalTerminal = flight[11];
            res.reservationFor.arrivalTime = JsonLd.toDateTime(flight[12] + flight[13], "dd MMMM yyyyhh:mm", ["en", "de"]);

            res.reservationNumber = flight[14];
            reservations.push(res);
        }
        return reservations;
    }
    else if (category.match(/Hotel/)) {
        let res = JsonLd.newLodgingReservation();
        const loc = ev.location.match(/^(.*?), (.*?), (.*), (.*)$/);
        res.reservationFor.name = loc[1];
        res.reservationFor.address.streetAddress = loc[2];
        res.reservationFor.address.addressLocality = loc[3];
        res.reservationFor.address.addressCountry = loc[4];
        res.checkinTime = ev.dtStart; // TODO this might break for date-only values?
        res.checkoutTime = ev.dtEnd;
        res.reservationNumber = ev.description.match(/code: (.*)\n/)[1];
        return res;
    }
    else if (category.match(/Rental|Mietwagen/)) {
        let res = JsonLd.newRentalCarReservation();
        const rental = ev.description.match(/^(.*)\n\S+ (.*?),.*, (.*?)\(.* (\d\d:\d\d) .* (\d+ \S+ \d{4}),.*\n\S+ (.*?),.*, (.*?)\(.* (\d\d:\d\d) .*, (\d+ \S+ \d{4}).*\n.*: (.*)\n.* (\S+)\n/);
        res.reservationFor.rentalCompany.name = rental[1];

        res.pickupLocation.name = rental[2];
        res.pickupLocation.address.streetAddress = rental[3];
        res.pickupTime = JsonLd.toDateTime(rental[5] + rental[4], "dd MMMM yyyyhh:mm", ["en", "de"]);

        res.dropoffLocation.name = rental[6];
        res.dropoffLocation.address.streetAddress = rental[7];
        res.dropoffTime = JsonLd.toDateTime(rental[9] + rental[8], "dd MMMM yyyyhh:mm", ["en", "de"]);

        res.reservationFor.name = rental[10];
        res.reservationNumber = rental[11];
        return res;
    }
    console.log("unhandled category", category);
}
