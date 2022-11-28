/*
   SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
   SPDX-License-Identifier: LGPL-2.0-or-later
*/

function parseConfirmationEmail(content) {
    const text = content.root.recursiveContent;

    let res = JsonLd.newLodgingReservation();
    res.reservationNumber = text.match(/Buchungs-Nr.: (.*?)[\s\n]/)[1];
    const timeRange = text.match(/Ankunft: (.*?) - Abreise: (.*)\n/);
    res.checkinDate = JsonLd.toDateTime(timeRange[1], 'dd. MMMM yyyy', 'de');
    res.checkoutDate = JsonLd.toDateTime(timeRange[2], 'dd. MMMM yyyy', 'de');
    res.underName.name = text.match(/Daten des Bestellers:\n(?:Herr|Frau) (.*)\n/)[1];
    const addr = text.match(/(.*)\n(.*)\n(.*)\nTel.: (.*)\nFax: .*\n(http.*)\n(.*@.*)\n/);
    res.reservationFor.name = addr[1];
    res.reservationFor.address.streetAddress = addr[2];
    res.reservationFor.address.addressLocality = addr[3];
    res.reservationFor.telephone = addr[4];
    res.reservationFor.url = addr[5];
    res.reservationFor.email = addr[6];
    return res;
}
