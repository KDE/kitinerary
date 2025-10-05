// SPDX-FileCopyrightText: 2025 David Pilarčík <meow@charliecat.space>
// SPDX-License-Identifier: LGPL-2.0-or-later

function parsePdf(pdf, node, triggerNode) {
    const page = pdf.pages[triggerNode.location];
	const results = []

	let [attendeeName, attendeeEmail] = page.textInRect(0.6, 0.3, 1, 0.4).split("\n")
	let bookingDate = page.text.match(/Creation date (\d{1,2} [A-z]{3} \d{4})/)[1]

	let isMultiEvent = !!page.text.match(/\d{1,2} [A-z]{3} \d{4} , \d{2}:\d{2} [AP]M - \d{2}:\d{2} [AP]M/)
	if (isMultiEvent) {
		let eventsListed = page.text.match(/.*\s+\d{1,2} [A-z]{3} \d{4} , \d{2}:\d{2} [AP]M - \d{2}:\d{2} [AP]M\s+/g)
		for (let eventText of eventsListed) {
			let res = JsonLd.newEventReservation();
			let [_e, eventName, dateStart, timeStart, dateEnd] = /(.*)\s+(\d+ [A-z]{3} \d+ ,) (\d+:\d+ [AP]M) - ((?:\d+ [A-z]{3} \d+ ,)? ?\d+:\d+ [AP]M)/g.exec(eventText)
			res.reservationFor.name = eventName
			res.reservationFor.startDate = JsonLd.toDateTime(dateStart + timeStart, "d MMM yyyy ,hh:mm A", "en");
			res.reservationFor.endDate = JsonLd.toDateTime(
				dateEnd.match(/\d+ [A-z]{3} \d+/) ? dateEnd : dateStart + dateEnd, 
				"d MMM yyyy ,hh:mm A", 
				"en"  
			);
			res.reservedTicket.ticketToken = "qrCode:" + triggerNode.content
			res.underName.name = attendeeName
			res.underName.email = attendeeEmail
			res.bookingTime = JsonLd.toDateTime(bookingDate, "d MMM yyyy", "en")
			res.reservationNumber = triggerNode.content

			results.push(res)
		}
	} else {
		let res = JsonLd.newEventReservation();
		let [_e, eventName, venueName, startDate, startTime, endTime] = /(.*)\s+Digital ticket, no need to print\s+(.*)\s+(\d+ [A-z]{3} \d+)\s+(\d+:\d+ [AP]M) - (\d+:\d+ [AP]M)\s+Booking summary/g.exec(page.text)
		res.reservationFor.name = eventName
		res.reservationFor.startDate = JsonLd.toDateTime(startDate + " " + startTime, "d MMM yyyy hh:mm A", "en");
		res.reservationFor.startDate = JsonLd.toDateTime(startDate + " " + endTime, "d MMM yyyy hh:mm A", "en");
		res.reservationFor.location.name = venueName
		res.reservedTicket.ticketToken = "qrCode:" + triggerNode.content
		res.underName.name = attendeeName
		res.underName.email = attendeeEmail
		res.bookingTime = JsonLd.toDateTime(bookingDate, "d MMM yyyy", "en")
		res.reservationNumber = triggerNode.content
		ExtractorEngine.extractPrice(page.text, res);
		results.push(res)
	}

	console.log(page.text)

	return results
}