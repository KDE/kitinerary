/*
   SPDX-FileCopyrightText: 2024 Kai Uwe Broulik <kde@broulik.de>
   SPDX-License-Identifier: LGPL-2.0-or-later
*/

function parsePkPass(pass, node) {
    let res = node.result[0];

    res.reservationFor.name = pass.field.eventname.value;
    res.reservationFor.startDate = JsonLd.toDateTime(pass.field.date.value, 'dd.MM.yyyy hh:mm', 'de');

    res.reservationFor.location = JsonLd.newObject("Place");
    res.reservationFor.location.name = pass.field.location.value;

    const positionField = pass.field.position;
    if (positionField && positionField.label === "Bereich / Reihe / Platz") {
        const positionSegments = positionField.value.split(" / ");
        res.reservedTicket.ticketedSeat = JsonLd.newObject("Seat");
        res.reservedTicket.ticketedSeat.seatSection = positionSegments[0];
        res.reservedTicket.ticketedSeat.seatRow = positionSegments[1];
        res.reservedTicket.ticketedSeat.seatNumber = positionSegments[2];
    }

    return res;
}
