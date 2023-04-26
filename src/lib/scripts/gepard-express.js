/*
   SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
   SPDX-License-Identifier: LGPL-2.0-or-later
*/

function parsePdf(pdf, node, triggerNode) {
    const text = pdf.pages[triggerNode.location].text;
    const legs = text.split(/  +(?=.* ->)/).slice(1);
    let reservations = [];
    for (leg of legs) {
        let res = JsonLd.newBusReservation();
        const stops = leg.match(/(.*) -> (.*)/);
        res.reservationFor.departureBusStop.name = stops[1];
        res.reservationFor.arrivalBusStop.name = stops[2];
        const times = leg.match(/ (\d{1,2}\.\d{1,2}\.\d{4} \d\d:\d\d)\n[\s\S]+ (\d{1,2}\.\d{1,2}\.\d{4} \d\d:\d\d)\n/);
        res.reservationFor.departureTime = JsonLd.toDateTime(times[1], 'dd.M.yyyy hh:mm', 'cz');
        res.reservationFor.arrivalTime = JsonLd.toDateTime(times[2], 'dd.M.yyyy hh:mm', 'cz');
        res.reservationFor.departurePlatform = leg.match(/Platform +(.*)/)[1];
        res.reservationNumber = triggerNode.content.match(/s:4:"code";s:\d+:"(.*?)"/)[1];
        res.underName.name = triggerNode.content.match(/s:8:"fullName";s:\d+:"(.*?)"/)[1];
        res.reservedTicket.ticketToken = 'qrcode:' + triggerNode.content;
        reservations.push(res);
        const seat = leg.match(/(?:Seat|sedadla:) +(.*?) \(/);
        if (seat)
            res.reservedTicket.ticketedSeat.seatNumber = seat[1];
    }
    return reservations;
}
