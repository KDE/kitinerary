/*
   SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

function parseTicket(ticket, node) {
    if (node.result.length > 1) // not sure this can happen
        return;

    var res = node.result[0];

    // decode 118199 vendor block
    var block = ticket.block("118199");
    var json = JSON.parse(block.contentText);
    if (!res.reservationFor.trainNumber)
        res.reservationFor.trainNumber = json["Z"];

    return res;
}
