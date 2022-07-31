/*
   SPDX-FileCopyrightText: 2019-2022 Volker Krause <vkrause@kde.org>
   SPDX-License-Identifier: LGPL-2.0-or-later
*/

function parseTicket(ticket, node) {
    // U_TLAY block in UIC 918.3 container with type "EOSU",
    // up to 9 rows of content
    // 0: Traveler name
    // 1: Ticket name/tariff
    // 2: Area of validity
    // 3: Time range of validity
    // 4: empty?
    // 5: Traveler birth date
    // 6: empty? (optional)
    // 7: departure station (optional)
    // 8: arrival station (optional)

    var res = JsonLd.newTrainReservation();
    res.reservedTicket = node.result[0];

    res.underName.name = ticket.ticketLayout.text(0, 0, 72, 1);
    res.reservedTicket.name = ticket.ticketLayout.text(1, 0, 72, 1);
    const valid = ticket.ticketLayout.text(3, 0, 72, 1).match(/([\d\.: ]+)-([\d\.: ]+)/);
    res.reservedTicket.validFrom = JsonLd.toDateTime(valid[1], "dd.MM.yyyy hh:mm", "de");
    res.reservedTicket.validUntil = JsonLd.toDateTime(valid[2], "dd.MM.yyyy hh:mm", "de");

    if (ticket.ticketLayout.size.height <= 6) {
        res.reservedTicket.underName = res.underName;
        return res.reservedTicket;
    }

    res.reservationNumber = res.reservedTicket.ticketNumber;
    res.reservationFor.provider = res.reservedTicket.issuedBy;
    res.reservationFor.departureTime = res.reservedTicket.validFrom;
    res.reservationFor.departureStation.name = ticket.ticketLayout.text(7, 0, 72, 1);
    res.reservationFor.arrivalStation.name = ticket.ticketLayout.text(8, 0, 72, 1);
    return res;
}
