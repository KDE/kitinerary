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
