/*
   SPDX-FileCopyrightText: 2024 Kai Uwe Broulik <kde@broulik.de>
   SPDX-License-Identifier: LGPL-2.0-or-later
*/

function parsePkPass(pass, node) {
    let res = node.result[0];

    // Remove "Welcome to...".
    res.reservationFor.location.name = pass.organizationName;
    // Setting location name overrides default heuristic for name = description.
    res.reservationFor.name = pass.description;

    // With a bigger sample size, figure out if "a1-label" is always name.
    const nameField = pass.auxiliaryFields.find(item => item.label === "Inhaber");
    if (nameField) {
        res.underName = JsonLd.newObject("Person");
        res.underName.name = nameField.value;
    }

    // "b5".
    const ticketIdField = pass.backFields.find(item => item.label === "TicketID");
    if (ticketIdField) {
        res.reservationNumber = ticketIdField.value;
    }

    // "s1-label".
    const ticketTypeField = pass.secondaryFields.find(item => item.label === "Ticket-Art");
    if (ticketTypeField) {
        res.reservedTicket.name = ticketTypeField.value;
    }

    return res;
}
