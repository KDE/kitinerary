// SPDX-FileCopyrightText: 2025 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

function extractTicket(mail, node) {
    var results = new Array();

    const res = JsonLd.newEventReservation();

    const pdf = node.findChildNodes({ mimeType: "application/pdf", match: ".*", scope: "Descendants" })[0];
    const text = pdf.findChildNodes({ mimeType: "text/plain", match: ".*", scope: "Descendants"})[1].content;

    const name = text.match(/Event\W+([a-zA-Z0-9 ]+)/)[1];
    res.reservedTicket.name = name;
    res.reservationFor.name = name;

    const ticketNumber = pdf.findChildNodes({ mimeType: "text/plain", match: "[0-9a-zA-Z]{9}", scope: "Descendants" })[0].content;
    res.reservedTicket.ticketToken = "qrCode:" + ticketNumber;
    res.reservedTicket.ticketNumber = ticketNumber;

    let price = text.match(/(Gesamt|Total)\W+([0-9]+.[0-9]+) ([A-Z]+)/)
    res.reservedTicket.totalPrice = parseFloat(price[2]);
    res.reservedTicket.priceCurrency = price[3];

    const googleUrl = node.childNodes[0].content.match(/http:\/\/www\.google\.com\/calendar.*/)[0];
    const dates = googleUrl.match(/dates=([0-9]{8}T[0-9]{6})%2F([0-9]{8}T[0-9]{6})/);
    res.reservationFor.startDate = JsonLd.toDateTime(dates[1], "yyyyMMddThhmmss", "de")
    res.reservationFor.endDate = JsonLd.toDateTime(dates[2], "yyyyMMddThhmmss", "de")

    const location = text.match(/(Ort|Location)\W+(.*)/)[2];
    res.reservationFor.location.address = location;

    res.underName.name = text.match(/Name.*\n\W+(.*?)  /)[1]

    results.push(res);
    return results
}
