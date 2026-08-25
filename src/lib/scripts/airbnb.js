// SPDX-FileCopyrightText: 2026 David Pilarčík <meow@charliecat.space>
// SPDX-License-Identifier: LGPL-2.0-or-later
function parseJsonLdMail(text, node, trigger) {
	let htmlSisterNode = trigger.childNodes.find(x => x.mimeType == "text/html")
	let res =
		// node.result[0] // Would love to do this, but the jsonld found isnt passed to us yet
		JSON.parse(htmlSisterNode.content.eval('//script[contains(@type, "application/ld+json")]')[0].content)

	res.bookingAgent = { "@type": "Organization", name: "Airbnb" }
	res.provider = res.bookingAgent;
	res.reservationFor.url = /https:\/\/(?:[a-z]{2,3}.)airbnb.com\/rooms\/\d{8}/gm.exec(text)[0];
	res.modifyReservationUrl = `https://airbnb.com/alterations/${res.reservationNumber}?role=guest` 
	res.cancelReservationUrl = `https://airbnb.com/reservation/itinerary?code=${res.reservationNumber}&auto_focus_row=cancellation_visualization`
	
	return res
}