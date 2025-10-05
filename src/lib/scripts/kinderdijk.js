// SPDX-FileCopyrightText: 2025 David Pilarčík <meow@charliecat.space>
// SPDX-License-Identifier: LGPL-2.0-or-later

function parsePdf(pdf, node, triggerNode) {
    const page = pdf.pages[triggerNode.location];
	const res = JsonLd.newEventReservation();

	res.reservedTicket.ticketToken = 'qrCode:' + triggerNode.content;
	res.reservationNumber = triggerNode.content // shows up under the qrcode

	// in example: ["Waterbus ticket", "Adult"] or ["Entrance Adult", "Opening hours: 09:00 - 17:30"]
	let [ticketType, admisionCategory] = page.textInRect(0, 0.45, 0.6, 0.47).split("\n")
	if (!admisionCategory.includes("Opening hours:"))
		res.reservedTicket.name = admisionCategory
	else
		res.reservedTicket.name = ticketType
	res.reservationFor.name = "Kinderdijk - " + ticketType

	// Explains ticket details (i.e. boat ticket use instructions)
	res.reservationFor.description = page.textInRect(0, 0.47, 0.6, 0.5)

	// in example "Date of validity: August 19, 2025"
	let dateValidity = page.textInRect(0, 0.5, 0.6, 0.52).match(/Date of validity:\s*(.+)/);
	res.reservationFor.startDate = JsonLd.toDateTime(dateValidity[1], "MMMM d, yyyy", ["en", "nl"]);

	let [attendeeName, attendeeEmail] = page.textInRect(0, 0.53, 0.6, 0.55).split("\n")
	res.underName.name = attendeeName;
	res.underName.email = attendeeEmail;

	let [_, streetAddress, city, country, telephone] = page.textInRect(0.8, 0.8, 1, 0.94).split("\n")
	res.reservationFor.location.address.streetAddress = streetAddress
	res.reservationFor.location.address.addressLocality = city
	res.reservationFor.location.address.country = country
	res.provider = Object.assign(
		JsonLd.newObject("Organization"), 
		{ telephone }
	)

	ExtractorEngine.extractPrice(page.text, res);
	return res
}