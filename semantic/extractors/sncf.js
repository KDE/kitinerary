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

function main(text) {
    var reservations = new Array();
    var bookingRef = text.match(/DOSSIER VOYAGE : ([A-Z0-9]{6})/);

    var pos = 0;
    while (true) {
        var header = text.substr(pos).match(/  Départ \/ Arrivée.*\n/);
        if (!header)
            break;
        var index = header.index + header[0].length;

        var res = JsonLd.newObject("TrainReservation");
        res.reservationNumber = bookingRef[1];
        res.reservationFor = JsonLd.newObject("TrainTrip");

        var depLine = text.substr(pos + index).match(/\n  ([\w -]+?)  +(\d{2}\/\d{2}) à (\d{2}h\d{2})/);
        if (!depLine)
            break;
        index += depLine.index + depLine[0].length;
        res.reservationFor.departureStation = JsonLd.newObject("TrainStation");
        res.reservationFor.departureStation.name = depLine[1];
        // TODO determine the year (which is nowhere in the ticket!)
        res.reservationFor.departureTime = JsonLd.toDateTime(depLine[2] + '/' + 1970 + ' ' + depLine[3], "dd/MM/yyyy hh'h'mm", "fr");

        var arrLine = text.substr(pos + index).match(/\n  ([\w -]+?)  +(\d{2}\/\d{2}) à (\d{2}h\d{2})/);
        if (!arrLine)
            break;
        index += arrLine.index + arrLine[0].length;
        res.reservationFor.arrivalStation = JsonLd.newObject("TrainStation");
        res.reservationFor.arrivalStation.name = arrLine[1];
        // TODO year, see above
        res.reservationFor.arrivalTime = JsonLd.toDateTime(arrLine[2] + '/' + 1970 + ' ' + arrLine[3], "dd/MM/yyyy hh'h'mm", "fr");

        // parse seat, train number, etc from the text for one leg
        // since the stations are vertically centered, the stuff we are looking for might be at different
        // positions relative to them
        var legText = text.substring(pos + header.index + header[0].length, pos + index);
        var trainNumber = legText.match(/TRAIN N°(\d{3,4})/);
        if (trainNumber)
            res.reservationFor.trainNumber = trainNumber[1];
        var seatRes = legText.match(/VOITURE (\d+) - PLACE (\d+)/);
        if (seatRes) {
            // TODO seat reservation data model is missing
            console.warn("coach:", seatRes[1], "seat:", seatRes[2]);
        }

        reservations.push(res);
        if (index == 0)
            break;
        pos += index;
    }

    return reservations;
}
