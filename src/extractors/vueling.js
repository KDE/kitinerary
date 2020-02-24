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

    var lang = doc.eval('//a[@class="copyright-text"]')[0].attribute("href").match(/\/([a-z]{2})$/)[1];

    var elems = doc.eval("//td[@class=\"vuelo_confirmado_card__subheader\"]");
    for (var i = 0; i < elems.length; ++i) {
        var elem = elems[i];
        var detailsRoot = elem.parent.nextSibling;

        var res = JsonLd.newFlightReservation();
        res.reservationNumber = bookingRef;

        var airportName = detailsRoot.eval(".//td[@class=\"vuelo_confirmado_card_details--city salida\"]")[0];
        var iataCode = detailsRoot.eval(".//td[@class=\"vuelo_confirmado_card_details--iata\"]")[0];
        res.reservationFor.departureAirport.iataCode = iataCode.content;
        res.reservationFor.departureAirport.name = airportName.content;
        res.reservationFor.arrivalAirport.iataCode = iataCode.nextSibling.content;
        res.reservationFor.arrivalAirport.name = airportName.nextSibling.content;

        var time = detailsRoot.eval(".//td[@class=\"vuelo_confirmado_card_details--time\"]")[0];
        res.reservationFor.departureTime = JsonLd.toDateTime(elem.content + ' ' + time.content.replace('h', ''), "dddd, dd MMMM yyyy HH:mm", lang);
        res.reservationFor.arrivalTime = JsonLd.toDateTime(elem.content + ' ' + time.nextSibling.content.replace('h', ''), "dddd, dd MMMM yyyy HH:mm", lang);

        var flightNum = detailsRoot.recursiveContent.match(/\s([A-Z0-9]{2})(\d{1,4})\b/);
        res.reservationFor.flightNumber = flightNum[2];
        res.reservationFor.airline.iataCode = flightNum[1];

        reservations.push(res);
    }

    return reservations;
}
