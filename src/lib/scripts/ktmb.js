/*
   SPDX-FileCopyrightText: 2026 Ajinkya Dahale <dahale.a.p@gmail.com>
   SPDX-License-Identifier: LGPL-2.0-or-later
*/

function extractQrTicket(text) {
    let res = JsonLd.newTrainReservation();

    res.reservedTicket.ticketToken = 'qrcode:' + text;

    var div = text.match(/([0-9]*),[a-f0-9-]{36},(T[0-9]{12}),([^,]*),([^,]*),([^,]*),[^,]*,[^,]*,(.+)/);

    if (parseInt(div[1]) != 1) {
        // TODO: multi passenger support?
        return [];
    }

    res.reservationNumber = div[2];
    res.reservationFor.trainNumber = div[3];
    res.reservationFor.departureTime = JsonLd.toDateTime(div[6], "dd MMM yyyy hh:mm:ss ap", "en");
    res.reservedTicket.ticketedSeat.seatSection = div[4];
    res.reservedTicket.ticketedSeat.seatNumber = div[5];

    return res;
}

function extractPdfTicketWithQr(pdf, node, triggerNode) {
    let res = triggerNode.result;
    const text = pdf.pages[triggerNode.location].text;

    // FIXME: not working
    // TODO: not supposed to be there for just "adults"
    // This may include other classes such as "senior" or "student"
    for (let r of res)
        r.underName.name = text.match(/(.+) - Adult/)[1];

    return res;
}

function extractTextTicket(text) {
    let res = JsonLd.newTrainReservation();

    // FIXME: not working
    // TODO: not supposed to be there for just "adults"
    // This may include other classes such as "senior" or "student"
    res.underName.name = text.match(/(.+) - Adult/)[1];

    return res;
}

function extractPdfTicketWithoutQr(pdf, node, triggerNode) {
    let res = triggerNode.result;
    const text = pdf.pages[triggerNode.location].text;

    // FIXME: not working
    // TODO: not supposed to be there for just "adults"
    // This may include other classes such as "senior" or "student"
    for (let r of res)
        r.underName.name = text.match(/(.+) - Adult/)[1];

    return res;
}
