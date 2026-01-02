/*
  SPDX-FileCopyrightText: 2024 Volker Krause <vkrause@kde.org>
  SPDX-License-Identifier: LGPL-2.0-or-later
*/

function extractEvent(ev) {
    let res = JsonLd.newLodgingReservation();
    res.checkinTime = ev.dtStart;
    res.checkoutTime = ev.dtEnd;
    const subject = ev.summary.match(/(.*) - ([^,\n]*)(?:, ([A-Z\d]+))?/);
    res.underName.name = subject[1];
    res.reservationFor.name = subject[2];
    res.reservationNumber = subject[3];
    res.reservationFor.address.addressLocality = ev.location;
    const desc = ev.description.match(/Hotel \(.*\)\n.*\n(.*)\n.*\n(.*)\n\n.*: (.*)\n/);
    res.reservationFor.address.streetAddress = desc[1];
    res.reservationFor.address.postalCode = desc[2];
    res.reservationFor.telephone = '+' + desc[3];
    return res;
}

function extractPdf(pdf) {
    const text = pdf.text;

    let links = [];
    for (const page of pdf.pages) {
        for (const link of page.links) {
            links.push(link);
        }
    }

    let idx = 0;
    let linkIdx = 1;
    let reservations = [];
    while (true) {
        const hotel = text.substr(idx).match(/(\S.*\S)  +(#[0-9A-Z]+)\n/);
        if (!hotel)
            break;
        idx += hotel.index + hotel[0].length;

        let res = JsonLd.newLodgingReservation();
        res.reservationFor.name = hotel[1];
        res.reservationNumber = hotel[2];
        const dates = text.substr(idx).match(/CHECK-IN +CHECK-OUT\n+ *(\d{1,2}[ -][a-z]{3}[ -]\d{4}) +(\d{1,2}[ -][a-z]{3}[ -]\d{4})\n+.*\n+([\s\S]+?,[ \n][A-Z]{3})\n/);
        res.checkinTime = JsonLd.toDateTime(dates[1], ["d-MMM-yyyy", "d MMM yyyy"], ["en", "it"]);
        res.checkoutTime = JsonLd.toDateTime(dates[2], ["d-MMM-yyyy", "d MMM yyyy"], ["en", "it"]);

        const addr = dates[3].replace('\n', ' ').split(',');
        res.reservationFor.address.streetAddress = addr[0];
        res.reservationFor.address.addressLocality = addr[1];
        if (addr.length == 5)
            res.reservationFor.address.addressRegion = addr[addr.length - 3];
        res.reservationFor.address.postalCode = addr[addr.length - 2];
        res.reservationFor.address.addressCountry = addr[addr.length - 1];

        const loc = links[linkIdx++].url.match(/(-?\d+\.\d+),(-?\d+\.\d+)/);
        res.reservationFor.geo.latitude = loc[1];
        res.reservationFor.geo.longitude = loc[2];
        reservations.push(res);
    }
    return reservations;
}
