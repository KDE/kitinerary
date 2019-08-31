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

function main(text) {
    var res = JsonLd.newBusReservation();
    res.reservationNumber = text.match(/Reference number +(\w+)\n/)[1];
    var date = text.match(/Outward - ([\d\/]+) /)[1];
    var dep = text.match(/Departing: ([\d:]+) (.*?)\n/);
    res.reservationFor.departureBusStop.name = dep[2];
    res.reservationFor.departureTime = JsonLd.toDateTime(date + dep[1], "dd/MM/yyyyhh:mm", "en");
    var arr = text.match(/Arriving: ([\d:]+) (.*?)\n/);
    res.reservationFor.arrivalBusStop.name = arr[2];
    res.reservationFor.arrivalTime = JsonLd.toDateTime(date + arr[1], "dd/MM/yyyyhh:mm", "en");

    // operator only serves locations in IE
    res.reservationFor.departureBusStop.address.addressCountry = "IE";
    res.reservationFor.arrivalBusStop.address.addressCountry = "IE";

    return res;
}
