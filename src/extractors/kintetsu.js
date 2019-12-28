/*
   Copyright (c) 2019 Luca Beltrame <lbeltrame@kde.org>

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

var reservationRegex = /Reservation number: (.*)/;
// Format: Station name(Station number)
var departureRegex = /Departure: (.*)(\(.*\))/;
var destinationRegex = /Arrival: (.*)(\(.*\))/;
var seatRegex = /Seat No.: (.*){1,}/;
var nameRegex = /To (.*)/;
var departureDateRegex = /Date of boarding: (.*?),(.*)/;
// Has a day of week as first item
var departureTimeRegex = /Departure time: (.*)/;
var arrivalTimeRegex = /Arrival time: (.*)/;
var coachRegex = /Car No.: (.*)/;

function main(text) {

    var res = JsonLd.newTrainReservation();
    res.reservationNumber = text.match(reservationRegex)[1];
    res.underName.name = text.match(nameRegex)[1];
    res.reservationFor.departureStation.name = text.match(departureRegex)[1];
    res.reservationFor.arrivalStation.name = text.match(destinationRegex)[1];

    var depDate = text.match(departureDateRegex)[2];
    var arrDate = text.match(departureDateRegex)[2];
    var depTime = text.match(departureTimeRegex)[1];
    var arrTime = text.match(departureTimeRegex)[1];

    var depDateTime = depDate + " " + depTime;
    var arrDateTime = arrTime + " " + arrTime;

    var seats = text.match(seatRegex)[1];

    res.reservationFor.departureTime = JsonLd.toDateTime(depDateTime, "dd MMM, yyyy hh:mm", "jp")
    res.reservationFor.arrivalTime = JsonLd.toDateTime(arrDateTime, "dd MMM, yyyy hh:mm", "jp")

    // TODO: Handle the fact that there are multiple seats but only one name with reservation
    res.reservedTicket.ticketedSeat.seatNumber = text.match(seatRegex)[1];
    res.reservedTicket.ticketedSeat.seatSection = text.match(coachRegex)[1];

    return res;

}



}
