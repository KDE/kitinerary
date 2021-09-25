/*
  SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
  SPDX-License-Identifier: LGPL-2.0-or-later
*/

function parseReservation(text)
{
    var reservations = new Array();
    const resNum = text.match(/予約番号\n(.*)\n/);
    const date = text.match(/乗車日.*?(\d.*)\n/);
    const trainInfo = text.match(/列車情報==([\s\S]*?)==/)[1];
    const legs = trainInfo.split(/\(\d列車目\)\n/);
    for (leg of legs) {
        console.log(leg);
        const fromTo = leg.match(/区　間：(.*?)\((.*?)\)→(.*?)\((.*?)\)\n/);
        if (!fromTo) {
            continue;
        }
        var res = JsonLd.newTrainReservation();
        res.reservationFor.departureStation.name = fromTo[1];
        res.reservationFor.departureStation.address.addressCountry = 'JP';
        res.reservationFor.departureTime = JsonLd.toDateTime(date[1] + fromTo[2], 'yyyy年MM月dd日hh時mm分', 'jp');
        res.reservationFor.arrivalStation.name = fromTo[3];
        res.reservationFor.arrivalStation.address.addressCountry = 'JP';
        res.reservationFor.arrivalTime = JsonLd.toDateTime(date[1] + fromTo[4], 'yyyy年MM月dd日hh時mm分', 'jp');

        const trainName = leg.match(/列車名：(.*)\n/);
        res.reservationFor.trainName = trainName[1];

        const seat = leg.match(/座　席：(.*号車)(.*)\n/);
        if (seat) {
            res.reservedTicket.ticketedSeat.seatSection = seat[1];
            res.reservedTicket.ticketedSeat.seatNumber = seat[2];
        }

        res.reservationNumber = resNum[1];
        reservations.push(res);
    }
    return reservations;
}
