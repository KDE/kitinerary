/*
   SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
   SPDX-License-Identifier: LGPL-2.0-or-later
*/

function parseBoardingPass(pdf, node, triggerNode) {
    var res = triggerNode.result[0];
    const page = pdf.pages[triggerNode.location];
    const timesText = page.textInRect(0.5, 0.5, 1, 1);
    const times = timesText.match(/\n(\d\d:\d\d)[\s\S]*?\n(\d\d:\d\d)[\s\S]*?\n(\d\d:\d\d)[\s\S]*?\n(\d\d:\d\d)/);
    res.reservationFor.boardingTime = JsonLd.toDateTime(times[2], "hh:mm", "en");
    res.reservationFor.departureTime = JsonLd.toDateTime(times[3], "hh:mm", "en");
    res.reservationFor.arrivalTime = JsonLd.toDateTime(times[4], "hh:mm", "en");
    return res;
}
