/*
   SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
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

    return res;
}
