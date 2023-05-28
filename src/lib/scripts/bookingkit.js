/*
   SPDX-FileCopyrightText: 2023 Kai Uwe Broulik <kde@broulik.de>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

function main(pass, node)
{
    let res = node.result[0];

    const eventName = pass.field["event"];
    res.reservationFor.name = eventName.value

    res.reservationFor.description = pass.description;

    res.reservationFor.location = JsonLd.newObject("Place");
    // This isn't technically correct but close enough most of the time.
    res.reservationFor.location.name = pass.organizationName;

    const eventTicketType = pass.field["event-ticket-type"];
    res.reservedTicket.name = eventTicketType.value;

    // Bookingkit returns UTC time but the displayed time is actually local.
    if (res.reservationFor.startDate.endsWith("Z")) {
        res.reservationFor.startDate = res.reservationFor.startDate.slice(0, -1); // chop(1)
    }

    return res;
}
