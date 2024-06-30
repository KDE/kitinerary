/*
   SPDX-FileCopyrightText: 2024 Luca Weiss <luca@lucaweiss.eu>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

function parseEmail(html) {
    var res = JsonLd.newTrainReservation();

    // Purchase code
    res.reservationNumber = html.root.recursiveContent.match(/Purchase code (\d+)/)[1];

    // Train details
    // XXX: This can probably have multiple rows, need sample
    const table1 = html.eval("//table")[0];
    const values1 = table1.eval(".//tr/td");
    if (values1.length != 14) {
        console.log("Unexpected table content");
        return null;
    }

    res.reservationFor.trainNumber = values1[7].content;
    res.reservationFor.departureStation.name = values1[8].content;
    res.reservationFor.departureTime = JsonLd.toDateTime(values1[9].content, "dd.MM.yyyy hh:mm", "ka");
    res.reservationFor.arrivalStation.name = values1[10].content;
    res.reservationFor.arrivalTime = JsonLd.toDateTime(values1[11].content, "dd.MM.yyyy hh:mm", "ka");
    res.reservedTicket.ticketedSeat.seatSection = values1[12].content;
    res.reservedTicket.ticketedSeat.seatNumber = values1[13].content;

    // Passenger information
    // XXX: This can probably have multiple rows, need sample
    const table2 = html.eval("//table")[1];
    const values2 = table2.eval(".//tr/td");
    if (values2.length != 8) {
        console.log("Unexpected table content");
        return null;
    }

    // values2[4]: Place type (e.g. "adult")
    res.underName.name = values2[5].content;
    // values2[6]: Document number (passport number)
    res.reservedTicket.totalPrice = values2[7].content;
    res.reservedTicket.priceCurrency = 'GEL';

    return res;
}
