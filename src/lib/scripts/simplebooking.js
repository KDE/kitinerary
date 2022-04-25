/*
   SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

function parseHtml(doc)
{
    var text = doc.root.recursiveContent;
    var res = JsonLd.newLodgingReservation();

    res.reservationNumber = text.match(/Booking #:\s*(.*?)\n/)[1];
    res.reservationFor.name = doc.eval("//h1")[0].content;

    res.checkinTime = JsonLd.toDateTime(text.match(/Arrival:\s*(.*?)\n/)[1], "dd MMM yyyy", "en");
    res.checkoutTime = JsonLd.toDateTime(text.match(/Departure:\s*(.*?)\n/)[1], "dd MMM yyyy", "en");

    res.reservationFor.geo.latitude = text.match(/Latitude:\s(-?\d+.\d+)/)[1] * 1.0;
    res.reservationFor.geo.longitude = text.match(/Longitude:\s(-?\d+.\d+)/)[1] * 1.0;

    var addr = text.match(/Lodging information[\n\s]+(.*?)\n[\n\s]+(.*?)\n[\n\s]+(.*?)\n[\n\s]+(?:Telep|P)hone: (.*?)\n.*\n*\s+Email:\s+(.*?)\n\s+Internet:\s+(.*?)\n/);
    res.reservationFor.address.streetAddress = addr[2];
    res.reservationFor.address.addressLocality = addr[3];
    res.reservationFor.telephone = addr[4];
    res.reservationFor.email = addr[5];
    res.reservationFor.url = addr[6];

    const guest = text.match(/Guest data[\n\s]+Name: (.*)/);
    res.underName.name = guest[1];

    return res;
}
