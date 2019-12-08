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

function main(content) {
    var res = JsonLd.newTrainReservation();

    var m = content.match(/PNR:(\d+),TRAIN:(.+?),DOJ:(.+?),TIME:(.+?),(.+?),(.+?) TO (.+?),(.*?)(\+\d)?,(.*?),/);
    res.reservationNumber = m[1];
    res.reservationFor.trainNumber = m[2];
    res.reservationFor.departureTime = JsonLd.toDateTime(m[3] + m[4], "dd-MM-yyyyhh:mm", "en");
    res.reservedTicket.ticketedSeat.seatingType = m[5];
    res.reservationFor.departureStation.name = m[6];
    res.reservationFor.departureStation.identifier = "ir:" + m[6];
    res.reservationFor.arrivalStation.name = m[7];
    res.reservationFor.arrivalStation.identifier = "ir:" + m[7];
    res.underName.name = m[8];
    res.reservedTicket.ticketedSeat.seatNumber = m[10];

    return res;
}
