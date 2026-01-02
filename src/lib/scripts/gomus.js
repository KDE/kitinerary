/*
   SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
   SPDX-License-Identifier: LGPL-2.0-or-later
*/

function parsePass(pass, node) {
    var event = node.result[0];

    if (pass.field['event-name'].label !== "Jahreskarte") {
        event.reservedTicket.name = pass.description;
        event.reservationFor.name = pass.field['event-name'].value;
        event.reservationFor.location.name = pass.field['location'].value;
        event.reservationFor.location.address = JsonLd.newObject('PostalAddress');
        event.reservationFor.location.address.streetAddress = pass.locations[0].relevantText.match(/ in (.*)\.$/)[1];
        return event;
    }

    var ticket = JsonLd.newObject("Ticket")
    for (const field of pass.secondaryFields) {
        if (field.label === "gültig ab") {
            ticket.validFrom = field.value;
            break;
        }
    }
    ticket.ticketNumber = pass.field['numbering'].value;
    ticket.validUntil = pass.expirationDate;
    ticket.issuedBy = JsonLd.newObject("Organization");
    ticket.issuedBy.name = pass.organizationName;
    ticket.name = pass.field['event-name'].value + " - " + pass.organizationName;
    ticket.ticketToken = event.reservedTicket.ticketToken;
    ticket.underName = JsonLd.newObject("Person");
    ticket.underName.name = pass.field['personalization'].value;
    return ticket;
}
