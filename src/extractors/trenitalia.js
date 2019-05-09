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

function parsePdf(pdf) {
    var reservations = new Array();

    for (var i = 0; i < pdf.pageCount; ++i) {
        var page = pdf.pages[i];
        var text = page.text;

        var res = JsonLd.newObject("TrainReservation");
        res.reservedTicket = JsonLd.newObject("Ticket");
        res.reservationFor = JsonLd.newObject("TrainTrip");

        var pnr = text.match(/PNR: (\S+)/);
        if (pnr) {
            res.reservationNumber = pnr[1];
        }

        var train = text.match(/(?:Train|Treno): (.*)\n/);
        if (!train) {
            break;
        }
        res.reservationFor.trainNumber = train[1];

        var times = text.match(/(?:Hours|Ore(?:\/Time)?) (\d{2}:\d{2}) - (\d{2}\/\d{2}\/\d{4}) +(?:Hours|Ore(?:\/Time)?) (\d{2}:\d{2}) - (\d{2}\/\d{2}\/\d{4})/)
        res.reservationFor.departureTime = JsonLd.toDateTime(times[2] + times[1], "dd/MM/yyyyhh:mm", "it");
        res.reservationFor.arrivalTime = JsonLd.toDateTime(times[4] + times[3], "dd/MM/yyyyhh:mm", "it");

        var header = text.match(/(?:Stazione di Arrivo|Arrival station)/);
        var dest = text.substr(header.index + header[0].length).match(/\n *((?:\w+\.? )*\w+\.?)  +((?:\w+.? )*\w+\.?)(?:  |\n)/);
        res.reservationFor.departureStation = JsonLd.newObject("TrainStation");
        res.reservationFor.departureStation.name = dest[1];
        res.reservationFor.arrivalStation = JsonLd.newObject("TrainStation");
        res.reservationFor.arrivalStation.name = dest[2];

        reservations.push(res);
    }

    return reservations;
}
