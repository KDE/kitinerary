/*
   SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

function parseRegistration(html) {
    var res = JsonLd.newEventReservation();
    res.reservationFor.name = html.eval('//h1')[0].content;
    const subtitle = html.eval('//h3')[0].content;
    if (subtitle)
        res.reservationFor.name += ' - ' + subtitle;
    res.reservationNumber = html.eval('//h3/span')[0].content.match(/#(.*)/)[1];
    res.modifyReservationUrl = html.eval('//a')[0].attribute('href');

    var time = html.eval('//div/div/p/strong/..')[0].content.match(/\((.*)\s+-\s+(.*)\)/);
    res.reservationFor.startDate = JsonLd.toDateTime(time[1], ["d MMM yyyy, hh:mm", "d MMM yyyy, hh:mm:ss"], "en");
    res.reservationFor.endDate = JsonLd.toDateTime(time[2], ["d MMM yyyy, hh:mm", "d MMM yyyy, hh:mm:ss"], "en");

    let dd = html.eval('//dl')[0].firstChild;
    while (!dd.nextSibling.isNull) {
        const label = dd.content;
        dd = dd.nextSibling;
        if (label.match(/First Name/)) {
            res.underName.givenName = dd.content;
        } else if (label.match(/Last Name/)) {
            res.underName.familyName = dd.content;
        }
        dd = dd.nextSibling;
    }

    return res;
}
