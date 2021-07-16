/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

function parsePdfTicket(pdf, node) {
    var reservations = new Array();
    for (page of pdf.pages) {
        const text = page.textInRect(0.125,0,0.9,0.5);
        var res = JsonLd.newEventReservation();
        res.reservationNumber = text.match(/Auftragsnummer:\s*(\d+)/)[1];
        const name = text.match(/gekauft von:.*\n\s*\d.*\d\s*(\S.*)\n/);
        res.underName.name = name[1];
        res.reservationFor.name = text.substr(name.index + name[0].length).match(/(.*)\n/)[1];
        const venue = text.match(/Veranstalter:\n.*\n(.*)\n(.*?)(?:  .*)?\n(.*)\n/);
        res.reservationFor.location.name = venue[1];
        res.reservationFor.location.address.streetAddress = venue[2];
        res.reservationFor.location.address.addressLocality = venue[3];
        const dt = text.substr(venue.index + venue[0].length).match(/\w*, (.*) Uhr/)[1];
        res.reservationFor.startDate = JsonLd.toDateTime(dt, 'dd. MMM yyyy, hh:mm', 'de');
        const entry = text.match(/Einlass ab: \w*, (.*) Uhr/)[1];
        res.reservationFor.doorTime = JsonLd.toDateTime(entry, 'dd. MMM yyyy, hh:mm', 'de');

        const barcodes = node.findChildNodes({ mimeType: "text/plain", match: ".{6,20}", scope: "Descendants" });
        res.reservedTicket.ticketToken = 'qrCode:' + barcodes[0].content;

        reservations.push(res);
    }
    return reservations;
}
