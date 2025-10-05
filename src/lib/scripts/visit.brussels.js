// SPDX-FileCopyrightText: 2025 David Pilarčík <meow@charliecat.space>
// SPDX-License-Identifier: LGPL-2.0-or-later

function parsePdf(pdf, node, triggerNode) {
    const page = pdf.pages[triggerNode.location];
	const res = JsonLd.newEventReservation();

	// in example: ["Atomium : Monday 25 August 2025 at 10:00 AM til 06:00 PM", 5477XXXX-XXXX3754]
	let [venueAndDate, reservationNumber] = page.textInRect(0, 0, 0.5, 0.1).split("\n");
	let [venue, time] = venueAndDate.split(" : ");
	res.reservationFor.location.name = venue;
	time = /(.*) at (.*) til (.*)/.exec(time); // in example "Monday 25 August 2025 at 10:00 AM til 06:00 PM"
	res.reservationFor.startDate = JsonLd.toDateTime(time[1] + " " + time[2], "dddd d MMMM yyyy hh:mm a", "en")
	res.reservationFor.endDate = JsonLd.toDateTime(time[1] + " " + time[3],  "dddd d MMMM yyyy hh:mm a", "en")
	res.reservationNumber = reservationNumber;
	res.reservedTicket.ticketToken = "qrCode:" + reservationNumber // shows the same as the header number

	// in example: ["ORDXXXX3754 : LAST First", "16,95 EUR (Adult)-Admission-"]
	let [orderDetails, priceAndTicketType] = page.textInRect(0.5, 0, 1, 0.1).split("\n")
	let [orderNumber, orderee] = orderDetails.split(" : ");
	let [_p, price, currency, admitionType, ticketType] = /(\d+,\d{2}) ([A-Z]{3}) \((.+)\)-(.+)-/.exec(priceAndTicketType);
	res.reservedTicket.name = admitionType
	res.underName.name = orderee;

	res.reservationFor.name = page.textInRect(0, 0.16, 1, 0.25)

	let orderDate = page.text.match(/ORDER: ORD\d+ - (\d{4}-\d{2}-\d{2} \d{2}:\d{2}:\d{2})/)[1];
	res.reservationFor.bookingTime = JsonLd.toDateTime(orderDate, "yyyy-MM-dd hh:mm:ss", "en");

	ExtractorEngine.extractPrice(page.text, res);
	return res
}