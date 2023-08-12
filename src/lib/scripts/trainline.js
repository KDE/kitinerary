/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

function parseEvent(event) {
    let res = JsonLd.newTrainReservation();
    res.reservationFor.departureTime = JsonLd.readQDateTime(event, 'dtStart');
    res.reservationFor.arrivalTime = JsonLd.readQDateTime(event, 'dtEnd');
    res.reservationFor.departureStation.name = event.location;
    res.reservationFor.arrivalStation.name = event.summary.match(/.* → (.*) \[/)[1];
    return res;
}

function fixBrokenSncfTicketTokens(mail, node)
{
    // fix JSON-LD in mail bodies containing messed up SNCF ticket tokens
    // and invalid train numbers
    const htmlNode = node.findChildNodes({ scope: "Children", mimeType: "text/html"})[0];
    const pdfNodes = node.findChildNodes({ scope: "Children", mimeType: "application/pdf"});
    if (pdfNodes.length == 0 || pdfNodes[0].result.length == 0 || htmlNode.result.length == 0) {
        return;
    }

    let res = [];
    for (pdfNode of pdfNodes) {
        res = res.concat(pdfNode.result)
    }
    for (r of htmlNode.result) {
        if (r.reservedTicket.ticketToken.match(/^aztecCode:i0CV/i)) {
            r.reservedTicket.ticketToken = undefined;
            r.reservationFor.trainNumber = undefined;
        }
        // clear price information from HTML schema.org, as that's the trip total
        // while we get sub-prices from each PDF
        r.totalPrice = undefined;
        res.push(r);
    }
    return res;
}
