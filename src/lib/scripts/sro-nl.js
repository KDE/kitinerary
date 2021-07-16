/*
   SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
   SPDX-License-Identifier: LGPL-2.0-or-later
*/

function parseTicket(html, node) {
    var res = JsonLd.newEventReservation();
    var ticketNode = html.eval('//img/../../../../..')[1];
    res.reservationFor.name = ticketNode.recursiveContent.match(/(.*)\n/)[1];
    const text = html.root.recursiveContent;
    const dt = text.match(/Geldig op\s+(\d{2}-\d{2}-\d{4}) +(\d+:\d{2}) *- *(\d+:\d{2})/);
    res.reservationFor.startDate = JsonLd.toDateTime(dt[1] + dt[2], "dd-MM-yyyyhh:mm", "nl");
    res.reservationFor.endDate = JsonLd.toDateTime(dt[1] + dt[3], "dd-MM-yyyyhh:mm", "nl");
    res.reservationNumber = text.match(/nummer (\d{10}) /)[1];
    res.underName.name = text.match(/naam van\s+(.+)\n/)[1];

    const barcode = node.findChildNodes({ mimeType: "text/plain", match: "[^\s]", scope: "Descendants" })[0];
    res.reservedTicket.ticketToken = 'qrCode:' + barcode.content;
    return res;
}
