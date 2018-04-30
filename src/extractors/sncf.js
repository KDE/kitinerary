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

function parseDate(dateStr, timeStr) {
    // the text does not contain the year at all, so guess that from Context.senderDate
    var date = JsonLd.toDateTime(dateStr + '/' + Context.senderDate.getFullYear() + ' ' + timeStr, "dd/MM/yyyy hh'h'mm", "fr");
    if (date < Context.senderDate)
        date.setFullYear(Context.senderDate.getFullYear() + 1);
    return date;
}

function main(text) {
    var reservations = new Array();
    var bookingRef = text.match(/DOSSIER VOYAGE : +([A-Z0-9]{6})/);

    var pos = 0;
    while (true) {
        var header = text.substr(pos).match(/ +Départ \/ Arrivée.*\n/);
        if (!header)
            break;
        var index = header.index + header[0].length;

        var res = JsonLd.newObject("TrainReservation");
        res.reservationNumber = bookingRef[1];
        res.reservationFor = JsonLd.newObject("TrainTrip");

        var depLine = text.substr(pos + index).match(/\n {2,3}([\w -]+?)  +(\d{2}\/\d{2}) à (\d{2}h\d{2})/);
        if (!depLine)
            break;
        index += depLine.index + depLine[0].length;
        res.reservationFor.departureStation = JsonLd.newObject("TrainStation");
        res.reservationFor.departureStation.name = depLine[1];
        res.reservationFor.departureTime = parseDate(depLine[2],  depLine[3]);

        var arrLine = text.substr(pos + index).match(/\n {2,3}([\w -]+?)  +(\d{2}\/\d{2}) à (\d{2}h\d{2})/);
        if (!arrLine)
            break;
        index += arrLine.index + arrLine[0].length;
        res.reservationFor.arrivalStation = JsonLd.newObject("TrainStation");
        res.reservationFor.arrivalStation.name = arrLine[1];
        res.reservationFor.arrivalTime = parseDate(arrLine[2], arrLine[3]);

        // parse seat, train number, etc from the text for one leg
        // since the stations are vertically centered, the stuff we are looking for might be at different
        // positions relative to them
        var legText = text.substring(pos + header.index + header[0].length, pos + index);
        var trainNumber = legText.match(/TRAIN N° ?(\d{3,4})/);
        if (trainNumber)
            res.reservationFor.trainNumber = trainNumber[1];
        var seatRes = legText.match(/(VOITURE|COACH) (\d+) - PLACE (\d+)/);
        if (seatRes) {
            res.reservedTicket = JsonLd.newObject("Ticket");
            res.reservedTicket.ticketedSeat = JsonLd.newObject("Seat");
            res.reservedTicket.ticketedSeat.seatSection = seatRes[2];
            res.reservedTicket.ticketedSeat.seatNumber = seatRes[3];
        }

        reservations.push(res);
        if (index == 0)
            break;
        pos += index;
    }

    return reservations;
}
