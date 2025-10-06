/*
   SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
   SPDX-License-Identifier: LGPL-2.0-or-later
*/

function parseBoardingPass(pdf, node, triggerNode) {
    var res = triggerNode.result[0];
    const page = pdf.pages[triggerNode.location];
    const timesText = page.textInRect(0.0, 0.0, 0.5, 0.5, 180);
    let times = timesText.match(/\n(?:\d\d:\d\d)[\s\S]*?\n(\d\d:\d\d)[\s\S]*?\n(\d\d:\d\d)[\s\S]*?\n(\d\d:\d\d)/);
    if (!times) {
        times = timesText.match(/\n(?:\d\d:\d\d)[\s\S]*?\n(\d\d:\d\d)[\s\S]*?\n(\d\d:\d\d)/);
    }
    res.reservationFor.boardingTime = JsonLd.toDateTime(times[1], "hh:mm", "en");
    res.reservationFor.departureTime = JsonLd.toDateTime(times[2], "hh:mm", "en");
    res.reservationFor.arrivalTime = JsonLd.toDateTime(times[3], "hh:mm", "en");
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
