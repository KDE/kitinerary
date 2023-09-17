/*
   SPDX-FileCopyrightText: 2019-2022 Volker Krause <vkrause@kde.org>
   SPDX-License-Identifier: LGPL-2.0-or-later
*/

function parseTicket(code, node) {
    // VorteilsCard
    if (code.ticketLayout && code.ticketLayout.type == "RCT2" && code.ticketLayout.text(0, 0, 50, 1).match(/VORTEILSCARD/i)) {
        let card = JsonLd.newObject("ProgramMembership");
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

    // decode 118199 vendor block
    let res = node.result[0];
    const block = code.block("118199");
    const json = JSON.parse(block.contentText);
    if (!res.reservationFor.trainNumber)
        res.reservationFor.trainNumber = json["Z"];

    return res;
}

function parsePage(pdf, node, triggerNode) {
    const details = pdf.pages[triggerNode.location].textInRect(0, 0.35, 0.65, 1);
    let reservations = [];
    let idx = 0;
    while (true) {
        const leg = details.substr(idx).match(/(\S.*\S)  +(\d\d\.\d\d\.\d{4}) +(\d\d:\d\d) +(.*)\n *(\S.*\S)  +(\d\d\.\d\d\.\d{4}) +(\d\d:\d\d)/);
        if (!leg)
            break;
        idx += leg.index + leg[0].length;

        let res = JsonLd.newTrainReservation();
        res.reservationFor.departureStation.name = leg[1];
        res.reservationFor.departureTime = JsonLd.toDateTime(leg[2] + ' ' + leg[3], 'dd.MM.yyyy hh:mm', 'de');
        res.reservationFor.trainNumber = leg[4];
        res.reservationFor.arrivalStation.name = leg[5];
        res.reservationFor.arrivalTime = JsonLd.toDateTime(leg[6] + ' ' + leg[7], 'dd.MM.yyyy hh:mm', 'de');
        res = JsonLd.apply(triggerNode.result[0], res);
        reservations.push(res);
    }

    if (reservations.length > 0)
        return reservations;

    // look for non-elided station names
    const text = pdf.pages[triggerNode.location].text;
    const legs = text.match(/VON +-> NACH.*\n.*? ([A-Z].*) +-> (.*?)  .*\n.*? ([\w\*].*) +-> (.*?)  /);
    if (!legs) { return triggerNode.result; }

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
