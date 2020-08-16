/*
   SPDX-FileCopyrightText: 2017 Volker Krause <vkrause@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

function parseAirport(airport, name)
{
    airport.name = name;
    if (name.startsWith("Brussels Airport"))
        airport.iataCode = "BRU"; // disambiguate Brussel airports
    return airport;
}

function main(html) {
    var reservations = new Array();
    var bookingRef = html.eval("//td[@class='confirmation']")[0].recursiveContent;

    var flightDetailsRoot = html.eval("//h2[text()='Flight details']")[0].nextSibling;
    if (!flightDetailsRoot)
        return null;

    while (!flightDetailsRoot.isNull && flightDetailsRoot.name != "h2") {
        if (flightDetailsRoot.name != "table") {
            flightDetailsRoot = flightDetailsRoot.nextSibling;
            continue;
        }

        var rows = flightDetailsRoot.eval("./tr");
        for (var i in rows) {
            var cell = rows[i].firstChild;
            if (cell.firstChild.isNull)
                continue;

            var res = JsonLd.newFlightReservation();
            res.reservationNumber = bookingRef;
            parseAirport(res.reservationFor.departureAirport, cell.firstChild.content);
            var depTime = cell.recursiveContent.match(/([0-9]{2} [A-Za-z]{3} [0-9]{4}),\s*([0-9]{2}:[0-9]{2})/);
            if (!depTime)
                continue;
            res.reservationFor.departureTime = JsonLd.toDateTime(depTime[1] + ' ' + depTime[2], "dd MMM yyyy hh:mm", "en");

            cell = cell.nextSibling;
            parseAirport(res.reservationFor.arrivalAirport, cell.firstChild.content);
            var arrTime = cell.recursiveContent.match(/([0-9]{2} [A-Za-z]{3} [0-9]{4}),\s*([0-9]{2}:[0-9]{2})/);
            if (!arrTime)
                continue;
            res.reservationFor.arrivalTime = JsonLd.toDateTime(arrTime[1] + ' ' + arrTime[2], "dd MMM yyyy hh:mm", "en");

            cell = cell.nextSibling;
            var airline = cell.recursiveContent.match(/([A-Z0-9]{2}) ([0-9]{3,4})\s*([A-Z][A-Za-z0-9 ]*)/);
            if (!airline)
                continue;
            res.reservationFor.airline.iataCode = airline[1];
            res.reservationFor.airline.name = airline[3];
            res.reservationFor.flightNumber = airline[2];
            reservations.push(res);
        }

        flightDetailsRoot = flightDetailsRoot.nextSibling;
    }

    return reservations;
}
