// SPDX-FileCopyrightText: 2026 David Pilarčík <meow@charliecat.space>
// SPDX-License-Identifier: LGPL-2.0-or-later

function parsePdfTicket(pdf, node, barcode) {
	const res = JsonLd.newEventReservation()
	const page = pdf.pages[barcode.location];
	// Ticketing portal produces 1 ticket for multiple entries, cant seperate these nicely

	res.reservedTicket.ticketToken = 'qrcode:' + barcode.content;
	res.reservationNumber = barcode.content
	// res.reservationFor.provider.name = page.textInRect(0.3, 0.15, 0.9, 0.2)
	res.reservationFor.name = page.textInRect(0.21, 0.20, 0.66, 0.24)

	// in the sample, the "visit time" field appers in the text first
	let [startDate, purchaseDate] = page.text.match(/\d{2}-\d{2}-\d{4} \d{2}:\d{2}/gm)
	res.reservationFor.startDate = JsonLd.toDateTime(
		// 07-08-2026 10:30
		startDate,
		["dd-MM-yyyy hh:mm"],
		["en", "es"]
	)
	res.bookingTime = JsonLd.toDateTime(
		// 07-08-2026 10:04 (GTM +02:00)
		purchaseDate,
		["dd-MM-yyyy hh:mm"],
		["en", "es"]
	)

	// can move up and down depending on the amount of tickets purchased and contained in this "invoice"
	res.underName.name = page.textInRect(0.21, 0.39, 0.49, 0.43)

	// Example
	// "
	// Catedral de Málaga
	// CIF: R2900679H
	// Email: jesus@artisplendore.com
	// Phone: +34617500582
	// Address: C.Molina Lario, S / N 29015 - Málaga, Málaga España
	// "
	let [_, venueName, venueMail, venuePhone, venueAddr] =
		/General information\s+(.*)\s+CIF: .*\s+Email: (.*)\s+Phone: (.*)\s+Address: (.*)/gm.exec(page.text)

	res.reservationFor.location.name = venueName
	res.reservationFor.location.address.streetAddress = venueAddr
	res.reservationFor.location.email = venueMail
	res.reservationFor.location.telephone = venuePhone
	
	return res
}