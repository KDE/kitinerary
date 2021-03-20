/*
   SPDX-FileCopyrightText: 2019 Luca Beltrame <lbeltrame@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
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
