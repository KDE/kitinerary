/*
   SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
   SPDX-License-Identifier: LGPL-2.0-or-later
*/

function extractBoardingPass(pdf, node, barcode) {
    const text = pdf.pages[barcode.location].text;
    let res = barcode.result[0];
    const times = text.match(/(\d\d:\d\d) +(\d\d:\d\d)\n/);
    res.reservationFor.boardingTime = JsonLd.toDateTime(times[1], "hh:mm", "en");
    res.reservationFor.departureTime = JsonLd.toDateTime(times[2], "hh:mm", "en");
    return res;
}
