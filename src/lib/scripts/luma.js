// SPDX-FileCopyrightText: 2024 David Pilarčík <meow@charliecat.space>
// SPDX-License-Identifier: LGPL-2.0-or-later

function parseEvent(event) {
    const res = JsonLd.newEventReservation();

    // https://lu.ma/check-in/evt-***************?pk=g-*************** // Ticket Token
    // https://lu.ma/       e/evt-***************?pk=g-*************** // Link in Description
    res.reservedTicket.ticketToken = 'qrCode:' + event.description.match(/(https:\/\/lu.ma\/e\/evt-[A-z0-9]*\?pk=g-[A-z0-9]*)/gm)[0].replace('/e/', '/check-in/')

    res.reservationFor.name = event.summary;
    res.reservationFor.startDate = JsonLd.readQDateTime(event, 'dtStart');
    res.reservationFor.endDate = JsonLd.readQDateTime(event, 'dtEnd');

    res.reservationFor.location.name = event.description.match(/\sAddress:\s(.*)\s/)[1]
    res.reservationFor.location.geo.latitude = event.geoLatitude
    res.reservationFor.location.geo.longitude = event.geoLongitude

    res.underName.name = event.attendees[0].name;
    res.underName.email = event.attendees[0].email;

    return res;
}

function parsePkPass(pass, node) {
    const res = node.result[0]

    res.underName = {
        "@type": "Person",
        email: pass.field["guest_email"].value,
        name: pass.field["guest_name"].value
    }

    return res
}