/*
   SPDX-FileCopyrightText: 2019-2022 Volker Krause <vkrause@kde.org>
   SPDX-License-Identifier: LGPL-2.0-or-later
*/

function parseTicket(ticket, node) {
    if (node.result.length > 1) // not sure this can happen
        return;

    var res = node.result[0];

    // decode 118199 vendor block
    const block = ticket.block("118199");
    const json = JSON.parse(block.contentText);
    if (!res.reservationFor.trainNumber)
        res.reservationFor.trainNumber = json["Z"];

    return res;
}

function parsePage(pdf, node, triggerNode) {
    if (triggerNode.parent.result.length > 2) // not sure this can happen
        return;

    // look for non-elided station names
    const text = pdf.pages[triggerNode.location].text;
    const legs = text.match(/VON +-> NACH.*\n.*? ([A-Z].*) +-> (.*?)  .*\n.*? ([\w\*].*) +-> (.*?)  /);
    if (!legs) { return triggerNode.result; }

    var reservations = [];
    var res = triggerNode.parent.result[0];
    res.reservationFor.departureStation.name = legs[1];
    res.reservationFor.arrivalStation.name = legs[2];
    reservations.push(res);
    if (triggerNode.parent.result.length == 2) {
        var res = triggerNode.parent.result[1];
        res.reservationFor.departureStation.name = legs[3];
        res.reservationFor.arrivalStation.name = legs[4];
        reservations.push(res);
    }

    return reservations;
}

function parseUic9183(code, node) {
    // VorteilsCard
    if (code.ticketLayout && code.ticketLayout.type == "RCT2" && code.ticketLayout.text(0, 0, 50, 1).match(/VORTEILSCARD/i)) {
        var card = JsonLd.newObject("ProgramMembership");
        card.programName = code.ticketLayout.text(0, 0, 50, 1);
        card.membershipNumber = code.ticketLayout.text(1, 1, 16, 1);
        card.member = JsonLd.newObject("Person");
        card.member.givenName = code.ticketLayout.text(1, 52, 19 ,1);
        card.member.familyName = code.ticketLayout.text(2, 52, 19 ,1);
        card.token = 'aztecbin:' + ByteArray.toBase64(code.rawData);
        card.validFrom = JsonLd.readQDateTime(code, 'validFrom');
        card.validUntil = JsonLd.readQDateTime(code, 'validUntil');
        return card.programName != undefined ? card : undefined;
    }
}
