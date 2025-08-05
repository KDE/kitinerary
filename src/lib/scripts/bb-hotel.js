/*
   SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
   SPDX-License-Identifier: LGPL-2.0-or-later
*/

function parseConfirmation(html) {
    const text = html.root.recursiveContent;

    let res = JsonLd.newLodgingReservation();
    res.reservationNumber = text.match(/(?:Buchungsnummer:|Booking no.) (.*)/)[1];
    const hotel = text.match(/Hotel\n(.*)\n(.*)\n(.*) - (.*) - (.*)/i);
    res.reservationFor.name = hotel[1];
    res.reservationFor.address.streetAddress = hotel[2];
    res.reservationFor.address.postalCode = hotel[3];
    res.reservationFor.address.addressLocality = hotel[4];
    res.reservationFor.address.addressCountry = hotel[5];
    const dates = text.match(/(\d\d\/\d\d\/\d{4}|\d\d \S{3} \d{4}) \(.* (\d{1,2}[AP]M|\d\d).*\)\n[\s\S]+\n(\d\d\/\d\d\/\d{4}|\d\d \S{3} \d{4}) \(.* (\d{1,2}[AP]M|\d\d).*\)/);
    res.checkinTime = JsonLd.toDateTime(dates[1] + ' ' + dates[2], ['dd/MM/yyyy hh', 'dd MMM yyyy hAP'], ['de', 'en']);
    res.checkoutTime = JsonLd.toDateTime(dates[3] + ' ' + dates[4], ['dd/MM/yyyy hh', 'dd MMM yyyy hAP'], ['de', 'en']);
    res.reservationFor.telephone = html.root.eval('//a[starts-with(@href, "tel:")]')[0].content;
    res.reservationFor.email = html.root.eval('//a[starts-with(@href, "mailto:")]')[0].content;
    res.modifyReservationUrl = html.root.eval('//a[contains(@href, "my-booking")]')[0].attribute('href');

    let reservations = [];
    let idx = 0;
    while (true) {
        const p = text.substr(idx).match(/(.*)\n[^€\n]*\n.* x \d+\n.*€\n/);
        if (!p)
            break;
        idx += p.index + p[0].length;
        let r = JsonLd.clone(res);
        r.underName.name = p[1];
        reservations.push(r);
    }

    return reservations.length ? reservations : res;
}
