/*
   SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

function main(pdf, node) {
    var res = [];
    for (barcode of node.findChildNodes({ scope: "Descendants", mimeType: "internal/iata-bcbp" })) {
        if (barcode.location == undefined)
            continue;
        res.push(parsePage(pdf.pages[barcode.location], barcode));
    }
    return res;
}

function parsePage(page, node)
{
    var res = node.result[0];
    var time = page.text.match(/Departing at\s+(\d{1,2}:\d{2}[AP]M)/);
    if (time)
        res.reservationFor.departureTime = JsonLd.toDateTime(time[1], "h:mmA", "en")
    time = page.text.match(/Arriving at:\s+(\d{1,2}:\d{2}[AP]M)/);
    if (time)
        res.reservationFor.arrivalTime = JsonLd.toDateTime(time[1], "h:mmA", "en")
    return res;
}
