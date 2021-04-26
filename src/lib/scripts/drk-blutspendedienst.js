/*
   SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
   SPDX-License-Identifier: LGPL-2.0-or-later
*/

function parseReservation(html) {
    var h4s = html.eval('//h4');
    var res = JsonLd.newEventReservation();
    res.reservationFor.name = h4s[0].content;
    var dt = h4s[1].content.match(/(\d+\.\d+\.\d+) - (\d+:\d+)/);
    res.reservationFor.startDate = JsonLd.toDateTime(dt[1] + dt[2], "dd.MM.yyyyhh:mm", "de");
    res.reservationFor.location.name = h4s[2].content;
    var addr = h4s[2].nextSibling.content.match(/(.*)\n+(.*)/);
    res.reservationFor.location.address.streetAddress = addr[1];
    res.reservationFor.location.address.addressLocality = addr[2];

    var links = html.eval('//a');
    res.modifyReservationUrl = links[links.length - 1].attribute('href');

    return res;
}
