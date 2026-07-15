/*
   SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
   SPDX-License-Identifier: LGPL-2.0-or-later
*/

function parseReservation(html) {
    let res = JsonLd.newEventReservation();
    res.reservationFor.location.address.addressCountry = 'DE';

    const h4s = html.eval('//h4');
    if (h4s.length > 0) {
        res.reservationFor.name = h4s[0].content;
        var dt = h4s[1].content.match(/(\d+\.\d+\.\d+) - (\d+:\d+)/);
        res.reservationFor.startDate = JsonLd.toDateTime(dt[1] + dt[2], "dd.MM.yyyyhh:mm", "de");
        res.reservationFor.location.name = h4s[2].content;
        var addr = h4s[2].nextSibling.content.match(/(.*)\n+(.*)/);
        res.reservationFor.location.address.streetAddress = addr[1];
        res.reservationFor.location.address.addressLocality = addr[2];

    } else {
        const text = html.root.recursiveContent;
        const ev = text.match(/\| (\d{2}\.\d{2}\.\d{4}).*(\d\d:\d\d).*\n(.*)\n(.*)\n(.*)\n(.*)/);
        res.reservationFor.startDate = JsonLd.toDateTime(ev[1] + ev[2], "dd.MM.yyyyHH:mm", "de");
        res.reservationFor.name = ev[3];
        res.reservationFor.location.name = ev[5];
        res.reservationFor.location.address.addressLocality = ev[4];
        res.reservationFor.location.address.streetAddress = ev[6];
    }
    const links = html.eval('//a');
    res.modifyReservationUrl = links[links.length - 1].attribute('href');
    return res;
}
