/*
   Copyright (c) 2017 Volker Krause <vkrause@kde.org>

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

var regExMap = new Array();
regExMap['en_US'] = new Array();
regExMap['en_US']['bookingRef1'] = /Airline booking reference: ([A-Z0-9]{6})/;
regExMap['en_US']['bookingRef2'] = /[Bb]ooking reference\s*:\s*([A-Z0-9]{6})/;
regExMap['en_US']['flightLine'] = /Flight:\s+([A-Z0-9]{2}) *([0-9]{2,4}), (.+)\n/;
regExMap['en_US']['date'] = /Date: *[A-Z][a-z]{2} (.*)\n/;
regExMap['en_US']['departure'] = /Departure: *([0-9]+:[0-9]+) *(.*) *\(([A-Z]{3})\)/;
regExMap['en_US']['arrival'] = /Arrival: *([0-9]+:[0-9]+) *(.*) *\(([A-Z]{3})\)/;
regExMap['en_US']['traveler'] = /Traveller:\s+(.+)\n/;
regExMap['sv_SE'] = new Array();
regExMap['sv_SE']['bookingRef1'] = /Flygbolagets bokningsreferens: ([A-Z0-9]{6})/;
regExMap['sv_SE']['bookingRef2'] = /[Bb]okningsreferens\s*:\s*([A-Z0-9]{6})/;
regExMap['sv_SE']['flightLine'] = /Flyg:\s+([A-Z0-9]{2}) *([0-9]{2,4}), (.+)\n/;
regExMap['sv_SE']['date'] = /Datum: *.{3} (.*)\n/;
regExMap['sv_SE']['departure'] = /Avgång: *([0-9]+:[0-9]+) *(.*) *\(([A-Z]{3})\)/;
regExMap['sv_SE']['arrival'] = /Ankomst: *([0-9]+:[0-9]+) *(.*) *\(([A-Z]{3})\)/;
regExMap['sv_SE']['traveler'] = /Resenär:\s+(.+)\n/;

function main(text) {
    var reservations = new Array();
    var traveler = new Array();

    for (var locale in regExMap) {
        var bookingRef = text.match(regExMap[locale]['bookingRef1']);
        if (!bookingRef)
            bookingRef = text.match(regExMap[locale]['bookingRef2']);
        if (!bookingRef)
            continue;

        var pos = 0;
        while (true) {
            var flightLine = text.substr(pos).match(regExMap[locale]['flightLine']);
            if (!flightLine)
                break;
            var index = flightLine.index + flightLine[0].length;

            var res = JsonLd.newObject("FlightReservation");
            res.reservationNumber = bookingRef[1];
            res.reservationFor = JsonLd.newObject("Flight");
            res.reservationFor.flightNumber = flightLine[2];
            res.reservationFor.airline = JsonLd.newObject("Airline");
            res.reservationFor.airline.iataCode = flightLine[1];
            res.reservationFor.airline.name = flightLine[3];

            var depDate = text.substr(pos + index).match(regExMap[locale]['date']);
            if (depDate)
                index += depDate.index + depDate[0].length;
            var depLine = text.substr(pos + index).match(regExMap[locale]['departure']);
            if (!depLine)
                break;
            index += depLine.index + depLine[0].length;
            res.reservationFor.departureTime = JsonLd.toDateTime(depDate[1] + " " + depLine[1], "d MMM yyyy hh:mm", locale);
            res.reservationFor.departureAirport = JsonLd.newObject("Airport");
            res.reservationFor.departureAirport.name = depLine[2];
            res.reservationFor.departureAirport.iataCode = depLine[3];

            var arrDate = text.substr(pos + index).match(regExMap[locale]['date']);
            var arrLine = text.substr(pos + index).match(regExMap[locale]['arrival']);
            if (!arrLine)
                break;
            if (!arrDate || arrDate.index > arrLine.index) // arrival date is sometimes optional
                arrDate = depDate;
            index += arrLine.index + arrLine[0].length;
            res.reservationFor.arrivalTime = JsonLd.toDateTime(arrDate[1] + " " + arrLine[1], "d MMM yyyy hh:mm", locale);
            res.reservationFor.arrivalAirport = JsonLd.newObject("Airport");
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
                var name = text.substr(pos).match(regExMap[locale]['traveler']);
                if (!name)
                    break;
                pos += name.index + name[0].length;
                traveler.push(name[1]);
            }
            break;
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
            r.underName = JsonLd.newObject("Person");
            r.underName.name = traveler[j];
            result.push(r);
        }
    }
    return result;
}
