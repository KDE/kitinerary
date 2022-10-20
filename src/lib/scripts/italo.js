/*
   SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
   SPDX-License-Identifier: LGPL-2.0-or-later
*/

function parseTicket(content, node, triggerNode) {
    const text = content.pages[triggerNode.location].text;
    let res = JsonLd.newTrainReservation();
    res.reservedTicket.ticketToken = 'aztec:' + triggerNode.content;
    res.reservationNumber = text.match(/  ([A-Z0-9]{6})  /)[1];
    const dt = text.match(/ (\d\d\/\d\d) /)[1];
    const leg = text.match(/Treno: (\d+) (\d\d:\d\d) (.*?) → (\d\d:\d\d) (.*)\n/);
    res.reservationFor.departureStation.name = leg[3];
    res.reservationFor.departureTime = JsonLd.toDateTime(dt + ' ' + leg[2], 'dd/MM hh:mm', 'en');
    res.reservationFor.arrivalStation.name = leg[5];
    res.reservationFor.arrivalTime = JsonLd.toDateTime(dt + ' ' + leg[4], 'dd/MM hh:mm', 'en');
    res.reservationFor.trainNumber = leg[1];

    let idx = 0;
    let reservations = [];
    while (true) {
        let pas = text.substr(idx).match(/ +\d\. (.*)  +Voiture (\d+) Place (\d+)\n/);
        if (!pas) {
            break;
        }
        let r = JsonLd.clone(res);
        r.underName.name = pas[1];
        r.reservedTicket.ticketedSeat.seatSection = pas[2];
        r.reservedTicket.ticketedSeat.seatNumber = pas[3];
        reservations.push(r);
        idx += pas.index + pas[0].length;
    }

    return reservations.length > 0 ? reservations : res;
}
