/*
   Copyright (c) 2018 Volker Krause <vkrause@kde.org>

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

function makeStation(name)
{
    var station = JsonLd.newObject("TrainStation");
    station.name = name;
    station.address = JsonLd.newObject("PostalAddress");
    station.address.addressCountry = "KR"; // Korail only serves Korea
    return station;
}

function main(html) {
    var bookingYear = html.eval("//table//table//table/tr[3]")[0].recursiveContent.match(/(\d{4}.)/)[1];
    if (!bookingYear)
        return null;

    var tab = html.eval("//table//table//table//table//tr")[1]; // 0 is the table header
    if (!tab || tab.isNull)
        return null;

    var reservations = new Array();
    while (!tab.isNull) {
        var res = JsonLd.newObject("TrainReservation");
        res.reservationFor = JsonLd.newObject("TrainTrip");

        var cell = tab.firstChild;
        var date = bookingYear + cell.recursiveContent;
        cell = cell.nextSibling;
        res.reservationFor.trainNumber = cell.recursiveContent;
        cell = cell.nextSibling;
        res.reservationFor.trainName = cell.recursiveContent;
        cell = cell.nextSibling;
        res.reservationFor.departureStation = makeStation(cell.recursiveContent);
        cell = cell.nextSibling;
        res.reservationFor.departureTime = JsonLd.toDateTime(date + " " + cell.recursiveContent, "yyyy년M월d일 hh:mm", "kr");
        cell = cell.nextSibling;
        res.reservationFor.arrivalStation = makeStation(cell.recursiveContent);
        cell = cell.nextSibling;
        res.reservationFor.arrivalTime = JsonLd.toDateTime(date + " " + cell.recursiveContent, "yyyy년M월d일 hh:mm", "kr");

        reservations.push(res);
        tab = tab.nextSibling;
    }

    return reservations;
}
