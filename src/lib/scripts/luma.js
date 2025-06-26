// SPDX-FileCopyrightText: 2024 David Pilarčík <meow@charliecat.space>
// SPDX-License-Identifier: LGPL-2.0-or-later

function parseEvent(event, node) {
    const res = Object.assign(JsonLd.newEventReservation(), node.result[0]);

    // https://lu.ma/check-in/evt-***************?pk=g-*************** // Ticket Token
    // https://lu.ma/       e/evt-***************?pk=g-*************** // Link in Description
	let eventUrl = event.description.match(/(https:\/\/lu.ma\/e\/evt-\S*\?pk=g-\S*)/gm)[0]
    res.reservedTicket.ticketToken = 'qrCode:' + eventUrl.replace('/e/', '/check-in/')
	res.reservationNumber = new URL(eventUrl).searchParams.get('pk').replace('g-', '') || undefined; // Extracting defacto reservation code from the URL
	res.reservationFor.url = eventUrl;

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

	res.reservationFor.url = pass.field["event_url"].value.match(/(https?:\/\/[^\s]+)"/)[1];
	res.reservationNumber = (new URL(res.reservedTicket.ticketToken.replace('qrCode:', ''))).searchParams.get('pk').replace('g-', ''); // Extracting defacto reservation code from the URL

    res.underName = {
        "@type": "Person",
        email: pass.field["guest_email"].value,
        name: pass.field["guest_name"].value
    }

    return res
}