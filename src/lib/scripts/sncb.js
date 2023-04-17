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

function parseInternationalPdf(pdf, node, triggerNode) {
    const page = pdf.pages[triggerNode.location].text;
    let reservations = [];
    let idx = 0;
    while (true) {
        const leg = page.substr(idx).match(/(\d\d\/\d\d\/\d{4}) (\d\d:\d\d) (.*?) +-> +(\d\d:\d\d) (.*?)  +(.*?)  +(.*?)  +(.*)\n/);
        if (!leg) {
            break;
        }
        idx += leg.index + leg[0].length;

        let res = JsonLd.clone(triggerNode.result[0]);
        res.reservationFor.departureTime = JsonLd.toDateTime(leg[1] + ' ' + leg[2], 'dd/MM/yyyy hh:mm', 'nl');
        res.reservationFor.departureStation.name = leg[3];
        res.reservationFor.arrivalTime = JsonLd.toDateTime(leg[1] + ' ' + leg[4], 'dd/MM/yyyy hh:mm', 'nl');
        res.reservationFor.arrivalStation.name = leg[5];
        res.reservationFor.trainNumber = leg[6];
        res.reservedTicket.ticketedSeat.seatSection = leg[7];
        res.reservedTicket.ticketedSeat.seatNumber = leg[8];
        reservations.push(res);
    }
    return reservations;
}
