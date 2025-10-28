/*
   SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
   SPDX-License-Identifier: LGPL-2.0-or-later
*/

function parseBoardingPass(pdf, node, triggerNode) {
    const page = pdf.pages[triggerNode.location];
    let res = triggerNode.result[0];

    const topRight = page.textInRect(0.5, 0.0, 1.0, 0.5);
    const depTimes = topRight.match(/(\d\d:\d\d).*(\d\d:\d\d)/);
    res.reservationFor.boardingTime = JsonLd.toDateTime(depTimes[1], "hh:mm", "en");
    res.reservationFor.departureTime = JsonLd.toDateTime(depTimes[2], "hh:mm", "en");

    const bottomRight = page.textInRect(0.0, 0.0, 0.5, 0.5, 180);
    const times = bottomRight.match(/(\d\d:\d\d)/g);
    if (times.length > 3 && times[times.length - 1] != depTimes[2])
        res.reservationFor.arrivalTime = JsonLd.toDateTime(times[times.length -1], "hh:mm", "en");

    return res;
}

function fixHtmlBooking(html, node) {
    let reservations = node.result;
    let iataCodeMap = {};
    // fix missing IATA codes in arrival airports on return trips
    for (r of reservations)
        iataCodeMap[r.reservationFor.departureAirport.name] = r.reservationFor.departureAirport.iataCode;
    for (r of reservations) {
        if (r.reservationFor.arrivalAirport.iataCode === "") {
            r.reservationFor.arrivalAirport.iataCode = iataCodeMap[r.reservationFor.arrivalAirport.name];
        }
    }

    return reservations;
}
