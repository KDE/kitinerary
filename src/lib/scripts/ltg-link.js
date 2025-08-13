/*
   SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
   SPDX-License-Identifier: LGPL-2.0-or-later
*/

// ticket code seems to start with the issue date as "yyMMdd", in case we ever need a more strict pattern for that
function parsePdfTicket(pdf, node, triggerNode) {
    const text = pdf.pages[triggerNode.location].textInRect(0, 0, 0.8, 1);
    let res = JsonLd.newTrainReservation();
    const stations = text.match(/FROM \/ TO\n? +(\S.*\S)  +(\S.*)/);
    res.reservationFor.departureStation.name = stations[1];
    res.reservationFor.arrivalStation.name = stations[2];
    res.reservationFor.provider.identifier = "uic:1124";
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

function extractPass(pass, node) {
    let res = node.result[0];
    res.reservationFor.departureStation = {
        '@type': 'TrainStation',
        name: pass.field['departure'].value
    };
    res.reservationFor.departureTime = JsonLd.readQDateTime(pass.field['departure_time'], 'value');
    res.reservationFor.arrivalStation = {
        '@type': 'TrainStation',
        name: pass.field['destination'].value
    };
    res.reservationFor.arrivalTime = JsonLd.readQDateTime(pass.field['arrival_time'], 'value');
    res.reservationFor.trainNumber = pass.field['line'].value;
    res.reservationFor.provider.identifier = "uic:1124";
    res.reservedTicket.ticketedSeat.seatSection = pass.field['vehicle'].value;
    res.reservedTicket.ticketedSeat.seatNumber = pass.field['seat'].value;
    const program = pass.field['campaign_ticket_code'];
    if (program) {
        res.programMembershipUsed = {
            '@type': 'ProgramMembership',
            programName: program.label,
            membershipNumber: program.value
        };
    }
    return res;
}
