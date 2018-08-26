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

function parseHtmlBooking(doc) {
    var reservations = new Array();

    var bookingRef = doc.eval("//td[@class=\"vuelo_confirmado_header\"]")[0].firstChild.content;
    if (!bookingRef)
        return null;

    var elems = doc.eval("//td[@class=\"vuelo_confirmado_card__subheader\"]");
    for (var i = 0; i < elems.length; ++i) {
        var elem = elems[i];
        var detailsRoot = elem.parent.nextSibling;

        var res = JsonLd.newObject("FlightReservation");
        res.reservationNumber = bookingRef;
        res.reservationFor = JsonLd.newObject("Flight");

        var airportName = detailsRoot.eval(".//td[@class=\"vuelo_confirmado_card_details--city salida\"]")[0];
        var iataCode = detailsRoot.eval(".//td[@class=\"vuelo_confirmado_card_details--iata\"]")[0];
        res.reservationFor.departureAirport = JsonLd.newObject("Airport");
        res.reservationFor.departureAirport.iataCode = iataCode.content;
        res.reservationFor.departureAirport.name = airportName.content;
        res.reservationFor.arrivalAirport = JsonLd.newObject("Airport");
        res.reservationFor.arrivalAirport.iataCode = iataCode.nextSibling.content;
        res.reservationFor.arrivalAirport.name = airportName.nextSibling.content;

        var time = detailsRoot.eval(".//td[@class=\"vuelo_confirmado_card_details--time\"]")[0];
        res.reservationFor.departureTime = JsonLd.toDateTime(elem.content + ' ' + time.content.replace('h', ''), "dddd, dd MMMM yyyy HH:mm", "es");
        res.reservationFor.arrivalTime = JsonLd.toDateTime(elem.content + ' ' + time.nextSibling.content.replace('h', ''), "dddd, dd MMMM yyyy HH:mm", "es");

        var flightNum = detailsRoot.eval(".//td[@class=\"v-middle vuelo_confirmado_card_details--numVuelo\"]")[0].content;
        res.reservationFor.flightNumber = flightNum.substr(2);
        res.reservationFor.airline = JsonLd.newObject("Airline");
        res.reservationFor.airline.iataCode = flightNum.substr(0, 2);

        reservations.push(res);
    }

    return reservations;
}
