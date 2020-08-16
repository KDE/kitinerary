/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

function parsePage(page) {
    var res = JsonLd.newTrainReservation();
    res.reservationFor.departureStation.name = page.text.match(/De:\s+(.*)\n/)[1];
    res.reservationFor.arrivalStation.name = page.text.match(/A:\s+(.*)\n/)[1];
    var date = page.text.match(/Date du voyage:\s+(\d\d)\/(\d\d)\/(\d{4})\n/);
    res.reservationFor.departureDay = date[3] + "-" + date[2] + "-" + date[1];
    res.underName.givenName = page.text.match(/Prénom:\s+(.*)\n/)[1];
    res.underName.familyName = page.text.match(/Nom:\s+(.*)\n/)[1];
    res.reservedTicket.ticketedSeat.seatingType = page.text.match(/Classe:\s+(.*)\n/)[1];
    res.reservedTicket.ticketToken = "barcode128:" + page.text.match(/\s+([A-Z\d-]{15})\n/)[1];
    return res;
}

function parsePdf(pdf) {
    var reservations = new Array();
    var pages = pdf.pages;
    for (var i = 0; i < pages.length; ++i) {
        reservations.push(parsePage(pages[i]));
    }
    return reservations;
}
