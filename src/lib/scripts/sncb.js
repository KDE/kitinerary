/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

function parsePage(page) {
    var res = JsonLd.newTrainReservation();
    res.reservationFor.departureStation.name = page.text.match(/(?:De|From):\s+(.*)\n/)[1];
    res.reservationFor.arrivalStation.name = page.text.match(/(?:A|TO):\s+(.*)\n/)[1];
    var date = page.text.match(/(?:Date du voyage|Travel date):\s+(\d\d)\/(\d\d)\/(\d{4})\n/);
    res.reservationFor.departureDay = date[3] + "-" + date[2] + "-" + date[1];
    res.underName.givenName = page.text.match(/(?:Prénom|Firstname):\s+(.*)\n/)[1];
    res.underName.familyName = page.text.match(/(?:Nom|Lastname):\s+(.*)\n/)[1];
    res.reservedTicket.ticketedSeat.seatingType = page.text.match(/Classe?:\s+(.*)\n/)[1];
    res.reservedTicket.ticketToken = "barcode128:" + page.text.match(/\s+([A-Z\d-]{15})\n/)[1];
    res.reservedTicket.name = page.text.match(/(?:Type de billet|Ticket type):\s+(.*)\n/)[1];
    const price = page.text.match(/(?:Prix|Price):\s+(\d+,\d\d)(?: €)?\n/);
    if (price) {
        res.totalPrice = price[1].replace(',', '.');
    }
    res.priceCurrency = 'EUR';
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

function parseRct2(uic, node) {
    const text = uic.ticketLayout.text(9, 0, 72, 1);
    const train = text.match(/(\d+) (\S.*\S) - (\S.*\S) (\d{2}\/\d{2}\/\d{4} \d\d:\d\d)/);
    let res = node.result[0];
    res.reservationFor.trainNumber = train[1];
    res.reservationFor.departureTime = JsonLd.toDateTime(train[4], "dd/MM/yyyy hh:mm", "be");
    return res;
}

function extractEmailBody(text) {
    let reservations = [];
    let idx = 0;
    while (true) {
        const date = text.substr(idx).match(/(?:\s+|.*: )\S+ (\d\d \S+ \d{4})\n/);
        if (!date)
            break;
        idx += date.index + date[0].length;
        while (true) {
            const leg = text.substr(idx).match(/^(\d\d:\d\d)[ \n](.*)\n(\d\d:\d\d)[ \n](.*)\n(\S+)\n/);
            if (!leg)
                break;
            idx += leg.index + leg[0].length;
            let res = JsonLd.newTrainReservation();
            res.reservationFor.departureTime = JsonLd.toDateTime(date[1] + ' ' + leg[1], 'dd MMM yyyy HH:mm', 'en');
            res.reservationFor.departureStation.name = leg[2];
            res.reservationFor.arrivalTime = JsonLd.toDateTime(date[1] + ' ' + leg[3], 'dd MMM yyyy HH:mm', 'en');
            res.reservationFor.arrivalStation.name = leg[4];
            res.reservationFor.trainNumber = leg[5];
            reservations.push(res);
        }
    }
    return reservations;
}

function mergeEmail(mail, node)
{
    console.log("XXX");
    const htmlResult = node.findChildNodes({ mimeType: "text/html", scope: "Children" })[0].result;
    let tickets = [];
    for (const ticket of node.findChildNodes({ mimeType: "internal/uic9183", scope: "Descendants"})) {
        tickets = tickets.concat(ticket.result);
    }
    if (htmlResult.length === 0 || tickets.length === 0)
        return;

    let result = [];
    for (const res of htmlResult) {
        for (const ticket of tickets) {
            if (!res.reservationFor.departureTime.startsWith(ticket.reservationFor.departureDay))
                continue;
            let r = JsonLd.clone(ticket);
            r.reservationFor = res.reservationFor;
            result.push(r);
        }
    }

    return result;
}
