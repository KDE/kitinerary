/*
   SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

function main(content, node) {
    // convert QR download links into the actual QR codes
    var res = node.result;
    for (var i = 0; i < res.length; ++i) {
        var ticketToken = res[i].reservedTicket.ticketToken;
        res[i].reservedTicket.ticketToken = ticketToken.replace(/^https?:\/\/api\.meinfernbus\.(..)\/qrcode\/..\//, "qrCode:https://shop.flixbus.$1/pdfqr/");

        // their schema.org annotations also claim train trips are bus trips, fix that
        if (res[i].reservationFor.departureBusStop.name.endsWith(" (FlixTrain)") && res[i].reservationFor.arrivalBusStop.name.endsWith(" (FlixTrain)")) {
            res[i] = JsonLd.busToTrainReservation(res[i]);
            res[i].reservationFor.departureStation.name = res[i].reservationFor.departureStation.name.substr(0, res[i].reservationFor.departureStation.name.length - 11);
            res[i].reservationFor.arrivalStation.name = res[i].reservationFor.arrivalStation.name.substr(0, res[i].reservationFor.arrivalStation.name.length - 11);
        }
    }
    return res;
}
