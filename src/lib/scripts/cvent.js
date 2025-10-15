// SPDX-FileCopyrightText: 2025 David Pilarčík <meow@charliecat.space>
// SPDX-License-Identifier: LGPL-2.0-or-later

function parsePkPass(pass, node) {
    let res = node.result[0];
    res.reservationFor.name = pass.field["title"].value;
    res.reservedTicket.name = pass.field["ticket_type"].value;
    res.reservationFor.startDate = pass.field["start_date"].value;
    res.reservationFor.location = Object.assign(
        JsonLd.newObject('Place'),
        {
            name: pass.field["location"].value,
        }
    )
    res.underName = Object.assign(
        JsonLd.newObject('Person'),
        {
            name: pass.field["attendee_name"]?.value
        }
    )
    res.reservationNumber = pass.barcodes[0].alternativeText;

    return res
}