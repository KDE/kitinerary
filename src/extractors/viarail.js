/*
    Copyright (C) 2020 Volker Krause <vkrause@kde.org>

    This program is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This program is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

function parseBoardingPass(pdf) {
    console.log(Context.barcode);

    // barcode content
    // 13x FTR number
    // 29x traveler family name
    // 2x coach number
    // 6x seat number
    // 4x departure station identifier
    // 4x arrival station identifier
    // 3x carrier identifier
    // 4x train number
    // 8x departure date yyyyMMdd
    // 4x departure time hhmm
    // 22x given name
    // 5x stuff, possibly containing Adult/Child/etc info
    // 6x confirmation number
    // 12x purchase date/time as yyyyMMddhhmm
    // 6x stuff, presumably class being part of this

    var res = JsonLd.newTrainReservation();
    res.reservationFor.provider = Context.barcode.substr(58, 3);
    res.reservationFor.trainNumber = Context.barcode.substr(61, 4).trim();
    res.reservationFor.departureTime = JsonLd.toDateTime(Context.barcode.substr(65, 12), "yyyyMMddhhmm", "en");
    res.reservationFor.departureStation.name = Context.barcode.substr(50, 4);
    res.reservationFor.arrivalStation.name = Context.barcode.substr(54, 4);
    res.reservedTicket.ticketedSeat.seatSection = Context.barcode.substr(42, 2).trim();
    res.reservedTicket.ticketedSeat.seatNumber = Context.barcode.substr(44, 6).trim();
    res.reservedTicket.ticketToken = "azteccode:" + Context.barcode;
    res.underName.familyName = Context.barcode.substr(13, 29).trim();
    res.underName.givenName = Context.barcode.substr(77, 22).trim();
    res.reservationNumber = Context.barcode.substr(104, 6);
    return res;
}
