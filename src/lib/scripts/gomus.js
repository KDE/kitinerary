/*
   SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
   SPDX-License-Identifier: LGPL-2.0-or-later
*/

function parsePass(pass, node) {
    var event = node.result[0];
    event.reservationFor.name = pass.field['event-name'].value;
    event.reservationFor.location.name = pass.field['location'].value;
    event.reservationFor.location.address = JsonLd.newObject('PostalAddress');
    event.reservationFor.location.address.streetAddress = pass.locations[0].relevantText.match(/ in (.*)\.$/)[1];
    event.reservedTicket.name = pass.description;
    return event;
}
