// SPDX-FileCopyrightText: 2026 David Pilarčík <meow@charliecat.space>
// SPDX-License-Identifier: LGPL-2.0-or-later

function parsePdfTicket(pdf, node, barcode) {
	const res = JsonLd.newEventReservation()
	const page = pdf.pages[barcode.location];
	
	res.reservedTicket.ticketToken = 'qrcode:' + barcode.content;
	res.reservedTicket.ticketNumber = barcode.content

	res.reservationNumber = /Localizador:\s+([A-Z0-9]{15})/gm.exec(page.text)[1]
	res.reservationFor.startTime = JsonLd.toDateTime(
		// "07/08/2026" dd/MM/yyyy
		/Fecha:\s+(\d{2}\/\d{2}\/\d{4})/gm.exec(page.text)[1],
		["dd/MM/yyyy"],
		"es"
	)
	res.reservationFor.location.name = /Lugar:\s+(.*)/gm.exec(page.text)[1]
	res.reservedTicket.name = /Tipo de entrada:\s+(.*)/gm.exec(page.text)[1]

	res.reservationFor.name = page.textInRect(0.07, 0.08, 0.33, 0.14)

	res.provider = Object.assign(
		JsonLd.newObject("Organization"),
		{ name: /Nombre: (.*)/.exec(page.textInRect(0.6, 0.8, 1.0, 0.9))[1] }
	)

	// TODO: fix detection for "Precio: 10,00 Eur."
	ExtractorEngine.extractPrice(page.text, res)
	
	return res
}