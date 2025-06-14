// SPDX-FileCopyrightText: ⓒ 2025 Volker Krause <vkrause@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

function extractConfirmation(text) {
    let res = JsonLd.newTrainReservation();
    const trip = text.match(/\S+ (\d{1,2} \S+ \d{4})\n[\S\s]+?Departs (.*) at (\d\d:\d\d)[\s\S]+?Arrives in (.*) at (\d\d:\d\d)\n/)
    res.reservationFor.departureStation.name = trip[2];
    res.reservationFor.departureTime = JsonLd.toDateTime(trip[1] + trip[3], "dd MMMM yyyyhh:mm", "en");
    res.reservationFor.arrivalStation.name = trip[4];
    res.reservationFor.arrivalTime = JsonLd.toDateTime(trip[1] + trip[5], "dd MMMM yyyyhh:mm", "en");
    res.reservationNumber = text.match("Booking Reference:\n(.*)\n")[1];
    ExtractorEngine.extractPrice(text.match(/Total paid.*[\s\n]+\d.*\n/)[0].replace(/[\s\n]+/g, ' '), res);
    return res;
}
