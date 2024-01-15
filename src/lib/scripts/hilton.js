/*
   SPDX-FileCopyrightText: 2024 Volker Krause <vkrause@kde.org>
   SPDX-License-Identifier: LGPL-2.0-or-later
*/

function extractHtml(html) {
    const text = html.root.recursiveContent;
    let res = JsonLd.newLodgingReservation();

    const r = text.match(/(#\d+)\n(.*)\n(.*)(?:  |\n)(.*)(?:, | )([A-Z]+)/);
    res.reservationNumber = r[1];
    res.reservationFor.name = r[2];
    res.reservationFor.address.streetAddress = r[3];
    res.reservationFor.address.addressLocality = r[4];
    res.reservationFor.address.addressCountry = r[5];
    res.reservationFor.telephone = html.root.eval('//a[starts-with(@href, "tel:")]')[0].attribute("href").substr(4);

    const dates = text.match(/([A-Z][a-z]{2}-\d{2}-\d{4}) - ([A-Z][a-z]{2}-\d{2}-\d{4})/);
    if (dates) {
        res.checkinTime = JsonLd.toDateTime(dates[1], "MMM-dd-yyyy", "en");
        res.checkoutTime = JsonLd.toDateTime(dates[2], "MMM-dd-yyyy", "en");
    }

    // calendar URL contains more structured data, prefer that if present
    const calHref = html.root.eval('//img[@alt="Add to Calendar"]/..')[0].attribute("href");
    const calData = new URL(calHref).searchParams.get("x").split('|');
    res.reservationFor.name = calData[1].length == 2 ? decodeURIComponent(calData[2]) : decodeURIComponent(calData[1]);
    res.reservationFor.address.streetAddress = decodeURIComponent(calData[3]);
    res.reservationFor.address.addressLocality = decodeURIComponent(calData[4]);
    res.reservationFor.address.addressRegion = decodeURIComponent(calData[5]);
    res.reservationFor.address.postcalCode = decodeURIComponent(calData[6]);
    res.reservationFor.address.addressCountry = decodeURIComponent(calData[7]);
    res.checkoutTime = JsonLd.toDateTime(decodeURIComponent(calData[9]) + decodeURIComponent(calData[8]), ["yyyy-MM-dd hh:mm:ss.zzz", "yyyy-MM-ddhh:mm"], "en");
    res.checkinTime = JsonLd.toDateTime(decodeURIComponent(calData[11]) + decodeURIComponent(calData[10]), ["yyyy-MM-dd hh:mm:ss.zzz", "yyyy-MM-ddhh:mm"], "en");
    res.reservatioNumber = calData[12];
    res.underName.name = calData[13];

    return res;
}
