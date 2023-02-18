/*
   SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
   SPDX-License-Identifier: LGPL-2.0-or-later
*/

function parseTicket(pdf, node, triggerNode) {
    const text = pdf.pages[triggerNode.location].text;
    var res = JsonLd.newTrainReservation();
    const header = text.match(/= +(\d{2}[ -][A-Z][a-z]{2}[ -]\d{4}) +(?:Out: |Ret: )?([A-Z]{3}) ?- ?([A-Z]{3})\n(.*)  +(.*)/);
    const date = header[1].replace(/-/g, ' ');
    const seat = text.match(/DEPART +COACH +SEAT\n(.*) +(\d{2}:\d{2}) +(.*?)  +(.*)\n/);
    const itinerary = text.match(/Itinerary.*\n +(.*)\n +(\d{2}:\d{2})\n +([\S\s]*?)\n +(\d{2}:\d{2})\n +(.*)/);

    res.reservationFor.departureStation.identifier = 'uk:' + header[2];
    res.reservationFor.arrivalStation.identifier = 'uk:' + header[3];
    if (itinerary) {
        res.reservationFor.departureStation.name = itinerary[1];
        res.reservationFor.departureTime = JsonLd.toDateTime(date + ' ' + itinerary[2], 'dd MMM yyyy hh:mm', 'en');
        res.reservationFor.arrivalStation.name = itinerary[5];
        res.reservationFor.arrivalTime = JsonLd.toDateTime(date + ' ' + itinerary[4], 'dd MMM yyyy hh:mm', 'en');
        res.reservationFor.trainName = itinerary[3].replace(/(.*?)(?:  .*)?(?:\n +|$)/g, '$1 ');
    } else {
        // unbound ticket
        res.reservationFor.departureStation.name = header[4];
        res.reservationFor.arrivalStation.name = header[5];
        res.reservationFor.departureDay = JsonLd.toDateTime(date, 'dd MMM yyyy', 'en');
    }
    if (seat && seat[4] != '*') {
        res.reservedTicket.ticketedSeat.seatSection = seat[3];
        res.reservedTicket.ticketedSeat.seatNumber = seat[4];
    }

    res.reservationNumber = text.match(/Ticket Number (.*)/)[1];
    res.reservedTicket.ticketToken = 'aztec:' + triggerNode.content;
    return res;
}
