/*
   SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
   SPDX-License-Identifier: LGPL-2.0-or-later
*/

function parseTicket(content, node, triggerNode) {
    const page = content.pages[triggerNode.location];
    const text = page.textInRect(0.0, 0.0, 0.5, 0.5);
    let res = JsonLd.newBoatReservation();
    res.reservedTicket.ticketToken = 'code39:' + triggerNode.content;
    res.reservationNumber = text.match(/TICKET ID\n\s+(\d+)/)[1];

    const dep = text.match(/Port of departure (.*)\n(?:.*\n)?\s*(\d{2}:\d{2} \d{2} \w+, \d{4})/);
    res.reservationFor.departureBoatTerminal.name = dep[1];
    res.reservationFor.departureBoatTerminal.geo = JsonLd.toGeoCoordinates(page.links[0].url);
    res.reservationFor.departureTime = JsonLd.toDateTime(dep[2], 'hh:mm dd MMMM, yyyy', 'it');

    const arr = text.match(/Port of arrival (.*)\n(?:.*\n)?\s*(\d{2}:\d{2} \d{2} \w+, \d{4})/);
    res.reservationFor.arrivalBoatTerminal.name = arr[1];
    res.reservationFor.arrivalTime = JsonLd.toDateTime(arr[2], 'hh:mm dd MMMM, yyyy', 'it');

    const passenger = page.textInRect(0.5, 0.0, 1.0, 0.5);
    res.underName.name = passenger.match(/(.*)\n/)[1];

    return res;
}
