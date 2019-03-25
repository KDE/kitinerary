/*
   Copyright (c) 2019 Volker Krause <vkrause@kde.org>

   This library is free software; you can redistribute it and/or modify it
   under the terms of the GNU Library General Public License as published by
   the Free Software Foundation; either version 2 of the License, or (at your
   option) any later version.

   This library is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
   FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
   License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to the
   Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
   02110-1301, USA.
*/

function parseEvent(event)
{
    var res;
    if (event.summary.match(/(?:Flight|Flug)/i)) {
        res = JsonLd.newObject("FlightReservation");
        res.reservationFor = JsonLd.newObject("Flight");
        res.reservationFor.airline = JsonLd.newObject("Airline");
        res.reservationFor.departureAirport = JsonLd.newObject("Airport");
        res.reservationFor.arrivalAirport = JsonLd.newObject("Airport");

        // force UTC, otherwise we lose the timezone due to JS converting to the local TZ
        res.reservationFor.departureTime = event.dtStart.toJSON();
        res.reservationFor.arrivalTime = event.dtEnd.toJSON();

        var flight = event.description.match(/(?:Flight no|Flugnr.):\s*(\w{2}) (\d{1,4})\n?.*(?:by|von): (.+)\n/);
        res.reservationFor.airline.name = flight[3];
        res.reservationFor.airline.iataCode = flight[1];
        res.reservationFor.flightNumber = flight[2];

        var from = event.description.match(/(?:From|Von):\s+(.*)\n/);
        res.reservationFor.departureAirport.name = from[1];

        var to = event.description.match(/(?:To|Nach):\s+(.*)\n/);
        res.reservationFor.arrivalAirport.name = to[1];

    } else if (event.summary.startsWith("Mietwagen")) {
        res = JsonLd.newObject("RentalCarReservation");
        res.reservationFor = JsonLd.newObject("RentalCar");
        res.reservationFor.rentalCompany = JsonLd.newObject("Organization");

        res.pickupLocation = JsonLd.newObject("Place");
        var pickup = event.description.match(/Abgabeort:\s*(.*)\n/);
        res.pickupLocation.name = pickup[1]; // TODO split address

        res.dropoffLocation = JsonLd.newObject("Place");
        var dropoff = event.description.match(/Annahmeort:\s*(.*)\n/);
        res.dropoffLocation.name = dropoff[1]; // TODO dito

        // force UTC, otherwise we lose the timezone due to JS converting to the local TZ
        res.pickupTime = event.dtStart.toJSON();
        res.dropoffTime = event.dtEnd.toJSON();

        var provider = event.description.match(/Mietwagenfirma:\s*(.*)\n/);
        res.reservationFor.rentalCompany.name = provider[1];
        var model = event.description.match(/Wagenkl.\/Typ:\s*(.*)\n/);
        res.reservationFor.name = model[1];
    } else {
        return null;
    }

    var refNum = event.description.match(/(?:Reservation code|Buchungsnummer|Buchungscode):\s(\w+)\n/);
    res.reservationNumber = refNum[1];

    return res;
}
