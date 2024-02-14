/*
   SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
   SPDX-License-Identifier: LGPL-2.0-or-later
*/

// ticket code seems to start with the issue date as "yyMMdd", in case we ever need a more strict pattern for that
function parsePdfTicket(pdf, node, triggerNode) {
    const text = pdf.pages[triggerNode.location].text;
    let res = JsonLd.newTrainReservation();
    const stations = text.match(/FROM \/ TO\n? +(\S.*\S)  +(\S.*)/);
    res.reservationFor.departureStation.name = stations[1];
    res.reservationFor.arrivalStation.name = stations[2];
    const dt = text.match(/(\d{4}-\d\d-\d\d)(?:\n.*){1,2}(\d\d:\d\d) - (\d\d:\d\d)/);
    res.reservationFor.departureTime = JsonLd.toDateTime(dt[1] + ' ' + dt[2], 'yyyy-MM-dd hh:mm', 'lt');
    res.reservationFor.arrivalTime = JsonLd.toDateTime(dt[1] + ' ' + dt[3], 'yyyy-MM-dd hh:mm', 'lt');
    const train = text.match(/SEAT\n *(\S.*?)  +(\S.*?)  +(\S.*?)  +(\S.*)\n/);
    res.reservationFor.trainNumber = train[1];
    res.reservedTicket.ticketedSeat.seatingType = train[2];
    res.reservedTicket.ticketedSeat.seatSection = train[3];
    res.reservedTicket.ticketedSeat.seatNumber = train[4];
    res.reservedTicket.ticketToken = 'qrcode:' + triggerNode.content;
    return res;
}
