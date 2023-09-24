/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

function parsePdf(pdf, node, triggerNode)
{
    if (!triggerNode.result) {
        return null;
    }

    var ptTicket = triggerNode.content
    var res = JsonLd.newEventReservation();
    res.reservationFor.name = ptTicket.ticketLayout.text(0, 52, 19, 1).trim();
    res.reservationFor.location.name = "Congress Center Leipzig";
    res.reservationFor.location.address.streetAddress = "Messeallee";
    res.reservationFor.location.address.postalCode = "04356";
    res.reservationFor.location.address.addressLocality = "Leipzig";
    res.reservationFor.location.address.addressCountry = "DE";
    res.reservationFor.location.geo.latitude = 51.39737;
    res.reservationFor.location.geo.longitude = 12.39528;
    var start = ptTicket.rct2Ticket.firstDayOfValidity;
    start.setHours(11);
    res.reservationFor.startDate = start;
    var end = ptTicket.rct2Ticket.firstDayOfValidity;
    end.setDate(30);
    end.setHours(18);
    res.reservationFor.endDate = end;

    // search for the QR code with the actual event code
    const barcodes = node.findChildNodes({ scope: "Descendants", mimeType: "text/plain", match: "^[0-9a-z]{40}$" });
    for (let i = 0; i < barcodes.length; ++i) {
        if (barcodes[i].location != triggerNode.location) {
            continue;
        }
        res.reservedTicket.ticketToken = "qrcode:" + barcodes[i].content;
    }

    // generate the second ticket for public transport
    var pt = JsonLd.clone(res);
    pt.reservedTicket.ticketToken = 'aztecbin:' + ByteArray.toBase64(triggerNode.content.rawData);
    pt.reservedTicket.name = "Public Transport";

    var reservations = new Array();
    reservations.push(res);
    reservations.push(pt);
    return reservations;
}
