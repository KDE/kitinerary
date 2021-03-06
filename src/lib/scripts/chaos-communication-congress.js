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
    res.reservationFor.name = ptTicket.person.name;
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
    var images = pdf.pages[triggerNode.location].images;
    for (var i = 0; i < images.length; ++i) {
        var code = Barcode.decodeQR(images[i]);
        if (code) {
            res.reservedTicket.ticketToken = "qrcode:" + code;
            break;
        }
    }

    // generate the second ticket for public transport
    var pt = JsonLd.clone(res);
    pt.reservedTicket.ticketToken = triggerNode.result[0].reservedTicket.ticketToken;
    pt.reservedTicket.name = "Public Transport";

    var reservations = new Array();
    reservations.push(res);
    reservations.push(pt);
    return reservations;
}
