/*
   SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

function parseEvent(event)
{
    var res;
    if (event.summary.match(/(?:Flight|Flug)/i)) {
        res = JsonLd.newFlightReservation();

        res.reservationFor.departureTime = JsonLd.readQDateTime(event, 'dtStart');
        res.reservationFor.arrivalTime = JsonLd.readQDateTime(event, 'dtEnd');

        var flight = event.description.match(/(?:Flight no|Flugnr.):\s*(\w{2}) (\d{1,4})\n?.*(?:by|von): (.+)\n/);
        res.reservationFor.airline.name = flight[3];
        res.reservationFor.airline.iataCode = flight[1];
        res.reservationFor.flightNumber = flight[2];

        var from = event.description.match(/(?:From|Von):\s+(.*)\n/);
        res.reservationFor.departureAirport.name = from[1];

        var to = event.description.match(/(?:To|Nach):\s+(.*)\n/);
        res.reservationFor.arrivalAirport.name = to[1];

    } else if (event.summary.startsWith("Mietwagen")) {
        res = JsonLd.newRentalCarReservation();

        var pickup = event.description.match(/Abgabeort:\s*(.*)\n/);
        res.pickupLocation.name = pickup[1]; // TODO split address

        var dropoff = event.description.match(/Annahmeort:\s*(.*)\n/);
        res.dropoffLocation.name = dropoff[1]; // TODO dito

        res.pickupTime = JsonLd.readQDateTime(event, 'dtStart');
        res.dropoffTime = JsonLd.readQDateTime(event, 'dtEnd');

        var provider = event.description.match(/Mietwagenfirma:\s*(.*)\n/);
        res.reservationFor.rentalCompany.name = provider[1];
        var model = event.description.match(/Wagenkl.\/Typ:\s*(.*)\n/);
        res.reservationFor.name = model[1];
    } else {
        return null;
    }

    var refNum = event.description.match(/(?:Reservation code|Buchungsnummer|Buchungscode):\s(.+)\n/);
    res.reservationNumber = refNum[1];

    return res;
}
