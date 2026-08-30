// SPDX-FileCopyrightText: 2026 David Pilarčík <meow@charliecat.space>
// SPDX-License-Identifier: LGPL-2.0-or-later

function parsePdfTicket(pdf, node, barcode) {
	const reservations = []
	
	for (let page of pdf.pages) {
		let passangerName = page.textInRect(0.5, 0.1, 0.8, 0.1)
		let ticketEmitedAt = /FECHA EMISIÓN BILLETE\s+(\d{2}\/\d{2}\/\d{4})/.exec(page.text)?.[1]

		for (let i = 1; i < 9; i++) {
			let top = (i / 10)
			let operatingCompanyName = page.textInRect(0.19, top, 0.40, top+0.04)
			if (!operatingCompanyName.includes("NOMBRE EMPRESA")) continue;

			let res = JsonLd.newBusReservation()
			res.reservationFor.provider = { "@type": "Organization", name: /NOMBRE EMPRESA\s+(.*)/gm.exec(operatingCompanyName)[1] }
			res.reservedTicket.name = page.textInRect(0.48, top+0.02, 0.57, top+0.04)
			res.reservationNumber = page.textInRect(0.57, top+0.02, 0.68, top+0.04)
			res.reservedTicket.ticketNumber = page.textInRect(0.68, top+0.02, 0.84, top+0.04)

			let [_1, departureBusStopName, arrivalBusStopName] = /(.*)\s+\/\s+(.*)/.exec(page.textInRect(0.10, top + 0.05, 0.41, top + 0.09)) ?? []
			res.reservationFor.departureBusStop.name = departureBusStopName
			res.reservationFor.arrivalBusStop.name = arrivalBusStopName

			let [_2, departDate, departTime, arriveDate, arriveTime] =
				/(\d{2}\/\d{2}\/\d{4})\s+(\d{2}:\d{2})\s+(?:(\d{2}\/\d{2}\/\d{4})\s+(\d{2}:\d{2}))?/gm.exec(page.textInRect(0.41, top + 0.05, 0.71, top + 0.09))
			res.reservationFor.departureTime = JsonLd.toDateTime(departDate + departTime, "dd/MM/yyyyhh:mm", "es")
			if (arriveDate && arriveTime) {
				res.reservationFor.arrivalTime = JsonLd.toDateTime(arriveDate + arriveTime, "dd/MM/yyyyhh:mm", "es")
			}

			res.reservationFor.busNumber = page.textInRect(0.71, top + 0.05, 0.77, top + 0.09)
			res.reservedTicket.ticketedSeat.seatNumber = page.textInRect(0.77, top + 0.05, 0.84, top + 0.09)
			res.underName.name = passangerName
			res.bookingTime = JsonLd.toDateTime(ticketEmitedAt, "dd/MM/yyyy", "es")

			let probbableTicketToken = page.imagesInRect(0.8, top, 1.0, top + 0.1)
			if (probbableTicketToken.length > 0) {
				let ticketToken = probbableTicketToken[0]
				res.reservedTicket.ticketToken = "qrcode:" + Barcode.decodeQR(ticketToken)
			}

			reservations.push(res)
		}
	}

	return reservations
}