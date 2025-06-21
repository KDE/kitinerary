/*
   SPDX-FileCopyrightText: ⓒ 2025 Volker Krause <vkrause@kde.org>
   SPDX-License-Identifier: LGPL-2.0-or-later
*/

function extractTrainPass(pass, node) {
    let res = JsonLd.newTrainReservation();
    res.reservedTicket = node.result[0].reservedTicket;
    res.reservationFor.provider = node.result[0].reservationFor.provider;
    res.underName = JsonLd.newObject("Person");
    res.underName.name = pass.field["header_0"].value;
    res.reservationNumber = pass.field["secondary_0"].value;
    res.reservationFor.departureStation.name = pass.field['primary_0'].label;
    res.reservationFor.departureDay = pass.field['auxiliary_0'].value;
    res.reservationFor.departureTime = pass.field['primary_0'].value;
    res.reservationFor.arrivalStation.name = pass.field['primary_1'].label;
    res.reservationFor.arrivalTime = pass.field['primary_1'].value;
    res.reservationFor.trainNumber = pass.field["auxiliary_1"].value;

    const stationCodes = res.reservedTicket.ticketToken.match(/\n\((\d{7})\) \S+\n\((\d{7})\) [^ ]+\n/);
    if (stationCodes) {
        res.reservationFor.departureStation.identifier = 'uic:' + stationCodes[1];
        res.reservationFor.arrivalStation.identifier = 'uic:' + stationCodes[2];
    }

    const seat = pass.field['auxiliary_2'].value.match(/^(\S+)\/(\S+)$/);
    if (seat) {
        res.reservedTicket.ticketedSeat = {
            '@type': 'Seat',
            seatSection: seat[1],
            seatNumber: seat[2]
        };
    }

    return res;
}
