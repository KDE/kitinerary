/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

function parseBoardingPass(pdf, node, triggerNode) {
    if (triggerNode.result.length != 1) return;
    const page = pdf.pages[triggerNode.location];
    const depText = page.textInRect(0.0, 0.2, 0.5, 0.4);
    const dep = depText.match(/DEPARTURE\n *(\d{4})\n *(\d{2}\w{3}\d{4})\n(.*\n)?(.*)\nFLIGHT/);
    const arrText = page.textInRect(0.5, 0.2, 1.0, 0.4);
    const arr = arrText.match(/ *ARRIVAL\n *(\d{4})\n *(\d{2}\w{3}\d{4})\n(.*\n)?(.*)\n *BOARDING TIME .*\n *(\d{4})/);

    let res = triggerNode.result[0];
    res.reservationFor.departureAirport.name = dep[4];
    res.reservationFor.departureTime = JsonLd.toDateTime(dep[2] + ' ' + dep[1], 'ddMMMyyyy hhmm', 'en');
    res.reservationFor.departureTerminal = dep[3];

    res.reservationFor.arrivalAirport.name = arr[4];
    res.reservationFor.arrivalTime = JsonLd.toDateTime(arr[2] + ' ' + arr[1], 'ddMMMyyyy hhmm', 'en');
    res.reservationFor.arrivalTerminal = arr[3];

    res.reservationFor.boardingTime = JsonLd.toDateTime(dep[2] + ' ' + arr[5], 'ddMMMyyyy hhmm', 'en');
    return res;
}
