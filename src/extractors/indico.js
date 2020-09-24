/*
   SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

function parseRegistration(html) {
    var res = JsonLd.newEventReservation();
    res.reservationFor.name = html.eval('//h1')[0].content;
    res.reservationNumber = html.eval('//h3/span')[0].content.match(/#(.*)/)[1];
    res.modifyReservationUrl = html.eval('//a')[0].attribute('href');

    var time = html.eval('//div/div/p/strong/..')[0].content.match(/\((.*)\s+-\s+(.*)\)/);
    res.reservationFor.startDate = JsonLd.toDateTime(time[1], "dd MMM yyyy, hh:mm", "en");
    res.reservationFor.endDate = JsonLd.toDateTime(time[2], "dd MMM yyyy, hh:mm", "en");

    return res;
}
