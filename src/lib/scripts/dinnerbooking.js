/*
   SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

function parseEvent(event)
{
    var res;
    res = JsonLd.newFoodEstablishmentReservation();
    res.reservationFor.name = event.organizer.name;
    res.startTime = JsonLd.readQDateTime(event, 'dtStart');
    res.endTime = JsonLd.readQDateTime(event, 'dtEnd');
    var addr = event.location.split(', ');
    res.reservationFor.address.streetAddress = addr[1];
    res.reservationFor.address.addressLocality = addr[2];

    var country = event.description.match(addr[2] + "\\n([\\w ]+)\\n\\n");
    if (country)
        res.reservationFor.address.addressCountry = country[1];

    var cancelUrl = event.description.match(/\n(https?:\/\/.*?\/cancel.*?)\n/);
    if (cancelUrl) {
        res.potentialAction = JsonLd.newObject("CancelAction");
        res.potentialAction.url = cancelUrl[1];
    }

    var url = event.description.match(/\n(https?:\/\/.+?)\n$/);
    if (url)
        res.reservationFor.url = url[1];

    if (event.attendees.length > 0) {
        res.underName.name = event.attendees[0].name;
        res.underName.email = event.attendees[0].email;
    }

    return res;
}
