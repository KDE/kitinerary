/*
   Copyright (c) 2017 Volker Krause <vkrause@kde.org>
   Copyright (c) 2018 Daniel Vrátil <dvratil@kde.org>

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

function isHeaderOrFooter(line) {
    return line.search(/(Jízdní řád( a rezervace)?|Jízdenku lze použít po stejné trase|Jízdní doklad zakoupený u obchodníka)/) >= 0;
}

function createSeat(res)
{
    if (!res.reservedTicket)
        res.reservedTicket = JsonLd.newObject("Ticket");
    if (!res.reservedTicket.ticketedSeat)
        res.reservedTicket.ticketedSeat = JsonLd.newObject("Seat");
}

function parseSeat(res, text) {
    var coach = text.match(/(\s+)(\d+)/);
    var idx = 0;
    if (coach) {
        createSeat(res);
        res.reservedTicket.ticketedSeat.seatSection = coach[2];
        idx = coach.index + coach[1].length + coach[2].length;
    }
    var seat = text.substr(idx).match(/\s+(\d+)/);
    if (seat) {
        createSeat(res);
        res.reservedTicket.ticketedSeat.seatNumber = seat[1];
    }
}

// There's no validity year anywhere in the ticket, so we take the purchase date and
// if the trip month and day are after the purchase month and day we assume the
// the ticket will become valid the same year it was purchased, otherwise we assume
// the ticket is for next year.
// This fails when you buy the ticket more than a year ahead of the trip, but I doubt
// you can even do that with Czech Railways...
function detectYear(tripDate, purchaseDate)
{
    var tripDay = parseInt(tripDate[1]);
    var tripMonth = parseInt(tripDate[2]);
    var purchaseDay = parseInt(purchaseDate[2]);
    var purchaseMonth = parseInt(purchaseDate[3]);
    var purchaseYear = parseInt(purchaseDate[4]);

    if ((purchaseMonth < tripMonth) ||
        (purchaseMonth === tripMonth) && (purchaseDay <= tripDay)) {
        return purchaseYear;
    } else {
        return purchaseYear + 1;
    }
}

function parseDeparture(res, line, purchaseDate) {
    res.reservationFor.departureStation = JsonLd.newObject("TrainStation");
    var station = line.match(/^(.+?)  /);
    if (!station)
        return;
    var idx = station.index + station[0].length;
    res.reservationFor.departureStation.name = station[1];
    var dt = line.substr(idx).match(/([0-9]{2})\.([0-9]{2})\. ([0-9]{2}:[0-9]{2})/);
    if (dt) {
        idx += dt.index + dt[0].length;
        res.reservationFor.departureTime = JsonLd.toDateTime(dt[1] + ' ' + dt[2] + ' ' + detectYear(dt, purchaseDate) + ' ' + dt[3], "dd MM yyyy hh:mm", "cs");
    }
    var trainId = line.substr(idx).match(/([a-zA-Z]+ [0-9a-zA-Z]+)/);
    if (trainId) {
        idx += trainId.index + trainId[0].length
        res.reservationFor.trainNumber = trainId[1];
    }
    parseSeat(res, line.substr(idx));
}

function parseArrival(res, line, purchaseDate) {
    res.reservationFor.arrivalStation = JsonLd.newObject("TrainStation");
    var station = line.match(/^(.+?)  /);
    if (!station)
        return;
    var idx = station.index + station[0].length;
    res.reservationFor.arrivalStation.name = station[1];
    var dt = line.substr(idx).match(/([0-9]{2})\.([0-9]{2})\. ([0-9]{2}:[0-9]{2})/);
    if (dt) {
        idx += dt.index + dt[0].length;
        res.reservationFor.arrivalTime = JsonLd.toDateTime(dt[1] + ' ' + dt[2] + ' ' + detectYear(dt, purchaseDate) + ' ' + dt[3], "dd MM yyyy hh:mm", "cs");
    }
}

function parseLegs(text, purchaseDate) {
    var reservations = new Array();
    var lines = text.split('\n');
    var depIdx = 1, arrIdx = 2;
    while (depIdx < lines.length) {
        // stop when reaching the footer or the next itinerary header
        if (isHeaderOrFooter(lines[depIdx]))
            return reservations;

        var res = JsonLd.newObject("TrainReservation");
        res.reservationFor = JsonLd.newObject("TrainTrip");

        arrIdx = depIdx + 1;
        parseDeparture(res, lines[depIdx], purchaseDate);
        parseArrival(res, lines[arrIdx], purchaseDate);
        depIdx = arrIdx + 1;
        // Find the next leg
        while (lines[depIdx].startsWith(" ")) {
            depIdx += 1;
        }

        reservations.push(res);
    }

    return reservations;
}

function main(text) {
    var reservations = new Array();
    var pos = 0;

    var purchaseDate = text.match(/([d|D]atum platby|UZP): ([0-9]{1,2})\.([0-9]{1,2})\.([0-9]{4})/)

    while (true) {
         // find itinerary headers
        var header = text.substr(pos).match(/Timetable( and Reservations)?/);
        if (!header)
            break;
        var idx = header.index + header[0].length;
        var timetableHeader = text.substr(pos + idx).match(/(Místo \/ Seat \/ Sitzplatz)/)
        idx = idx + timetableHeader.index + timetableHeader[0].length;
        reservations = reservations.concat(parseLegs(text.substr(pos + idx), purchaseDate));
        if (idx == 0)
            break;
        pos += idx + 1;
    }

    var bookingRef = text.match(/Kód transakce:\s*([A-Z0-9]{6})\n/);
    for (var i = 0; bookingRef && i < reservations.length; ++i)
        reservations[i].reservationNumber = bookingRef[1];
    return reservations;
}
