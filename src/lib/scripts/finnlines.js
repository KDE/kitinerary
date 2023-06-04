/*
   SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
   SPDX-License-Identifier: LGPL-2.0-or-later
*/

function parsePdfTicket(pdf, node, triggerNode) {
    let res = JsonLd.newBoatReservation();
    res.reservationNumber = triggerNode.content;
    const text = pdf.pages[triggerNode.location].text;
    const route = text.match(/Route (\S.*) - (\S.*?)  +/);
    res.reservationFor.departureBoatTerminal.name = route[1];
    res.reservationFor.arrivalBoatTerminal.name = route[2];
    const dt = text.match(/Departure (\S.*\d)  +Arrival (\S.*\d)/);
    res.reservationFor.departureTime = JsonLd.toDateTime(dt[1], 'ddd dd MMM yyyy hh:mm', 'en');
    res.reservationFor.arrivalTime = JsonLd.toDateTime(dt[2], 'ddd dd MMM yyyy hh:mm', 'en');
    return res;
}
