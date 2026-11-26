// SPDX-FileCopyrightText: 2026 David Pilarčík <meow@charliecat.space>
// SPDX-License-Identifier: LGPL-2.0-or-later
function parseJsonLdMail(html, node) {
    let res = node.result[0];

    res.bookingAgent = { "@type": "Organization", name: "Airbnb" }
    res.provider = res.bookingAgent;

    for (const elem of html.eval("//a[@href]")) {
        const link = elem.attribute("href");
        const url = link.match(/https:\/\/(?:[a-z]{2,3}.)airbnb.[a-z]{2,3}\/rooms\/\d{8,}/);
        if (url)
            res.reservationFor.url = url[0];
        if (link.match(/https:\/\/maps.google/))
            res.reservationFor.geo = JsonLd.toGeoCoordinates(link);
    }

    res.modifyReservationUrl = `https://airbnb.com/alterations/${res.reservationNumber}?role=guest`
    res.cancelReservationUrl = `https://airbnb.com/reservation/itinerary?code=${res.reservationNumber}&auto_focus_row=cancellation_visualization`

    return res
}
