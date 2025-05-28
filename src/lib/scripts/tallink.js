/*
   SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
   SPDX-License-Identifier: LGPL-2.0-or-later
*/

function parsePdfTicket(pdf, node, triggerNode) {
    let res = JsonLd.newBoatReservation();
    res.reservationNumber = triggerNode.content.match(/;(\d+)-/)[1];
    const text = pdf.pages[triggerNode.location].textInRect(0.0, 0.0, 0.68, 1.0);
    const dep = text.match(/(?:Abfahrt Von|Departure from) (\S.*?)  +\S.*(\d\d\.\d\d.\d{4}) +(.*)\n.*(\d\d:\d\d)  +(\S.*)/);
    res.reservationFor.departureBoatTerminal.name = dep[3];
    res.reservationFor.departureBoatTerminal.address.addressLocality = dep[1];
    res.reservationFor.departureBoatTerminal.address.streetAddress = dep[5];
    res.reservationFor.departureTime = JsonLd.toDateTime(dep[2] + ' ' + dep[4], 'dd.MM.yyyy hh:mm', 'de');

    const arr = text.match(/(?:Ankunft in|Arrives in) (\S.*?)  +\S.*(\d\d\.\d\d.\d{4}) +(.*)\n.*(\d\d:\d\d)  +(\S.*)/);
    res.reservationFor.arrivalBoatTerminal.name = arr[3];
    res.reservationFor.arrivalBoatTerminal.address.addressLocality = arr[1];
    res.reservationFor.arrivalBoatTerminal.address.streetAddress = arr[5];
    res.reservationFor.arrivalTime = JsonLd.toDateTime(arr[2] + ' ' + arr[4], 'dd.MM.yyyy hh:mm', 'de');

    return res;
}
