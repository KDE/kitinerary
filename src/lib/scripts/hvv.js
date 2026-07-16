// SPDX-FileCopyrightText: 2026 Volker Krause <vkrause@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

function extractPlatform(s)
{
    const p = s ? s.match(/Gleis (.*)/) : undefined;
    return p ? p[1] : undefined;
}

function extractEvent(ev) {
    let reservations = [];
    let idx = 0;
    while (true) {
        const leg = ev.description.substr(idx).match(/\) (.*) ➔ (.*)\nAb (\d\d:\d\d) (.*?)(?: \((.*)\))?\nAn (\d\d:\d\d) (.*?)(?: \((.*)\))?\n/);
        if (!leg)
            break;
        idx += leg.index + leg[0].length;
        let res = JsonLd.newTrainReservation();
        res.reservationFor.trainNumber = leg[1];
        res.reservationFor.departureDay = ev.dtStart;
        res.reservationFor.departureTime = leg[3];
        res.reservationFor.departureStation.name = leg[4];
        res.reservationFor.departurePlatform = extractPlatform(leg[5]);
        res.reservationFor.arrivalTime = leg[6];
        res.reservationFor.arrivalStation.name = leg[7];
        res.reservationFor.arrivalPlatform = extractPlatform(leg[8]);

        const busNum = leg[1].match(/(.*) \(Bus\)/);
        if (busNum) {
            res.reservationFor.trainNumber = busNum[1];
            res = JsonLd.trainToBusReservation(res);
        }

        reservations.push(res);
    }
    return reservations;
}
