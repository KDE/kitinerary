// SPDX-FileCopyrightText: 2026 David Pilarčík <meow@charliecat.space>
// SPDX-License-Identifier: LGPL-2.0-or-later

function parsePass(pass, node) {
	let res = Object.assign(JsonLd.newEventReservation(), node.result[0]); // having problems with the pass itself

	// Why is the value of the location field the event name?
	// Because for some reason the pass is structured like this:
	//	{
	// 		"key": "location",
	//		"label": "Venue Name",
	// 		"value": "Event title"
	//	}
	res.reservationFor.name = pass.field["location"].value;
	res.reservationFor.location.name = pass.field["location"].label;

	res.provider = { '@type': 'Organization', name: pass.organizationName };
	res.underName.name = pass.field["attendee"].value;
	res.reservedTicket.name = pass.field["ticket"].value;
	res.reservationFor.startDate = JsonLd.toDateTime(pass.field["date"].value, "d MMM, hh:mm a", "en");

	return res
}

function parsePDF(pdf, node, barcode) {
	let res = Object.assign(JsonLd.newEventReservation(), node.result[0]);
	let page = pdf.pages[0]; // unsure if they send multiple tickets as separate files, or in a single pdf

	res.reservedTicket.ticketToken = "qrCode:" + barcode.content;
	res.reservationFor.name = page.textInRect(0, 0.3, 1, 0.4) // Event Title
	res.reservationFor.location.name = page.textInRect(0, 0.4, 1, 0.45) // Venue Name, Bottova 2a, 811 09 Bratislava-Staré Mesto, Slovakia
	
	const dt = page.textInRect(0, 0.45, 1, 0.5).match(/\S+ (\d+).* (\S+)/);
	res.reservationFor.startDate = JsonLd.toDateTime(dt[1] + dt[2], "ddMMM", "en"); // Fri 19th Jun => "19Jun"
	res.reservedTicket.name = page.textInRect(0, 0.6, 0.30, 0.65) // is in a box "TICKET TYPE\nStandard", extracting only "Standard"
	res.underName.name = page.textInRect(0.3, 0.6, 0.60, 0.65) // Same as Above, but for the name

	res.reservationNumber = page.textInRect(0.6, 0.55, 1, 1)
	
	return res
}