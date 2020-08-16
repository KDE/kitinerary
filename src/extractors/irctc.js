/*
   SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
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
