// SPDX-FileCopyrightText: 2025 David Pilarčík <meow@charliecat.space>
// SPDX-License-Identifier: LGPL-2.0-or-later

function parsePkPass(pass, node) {
    const res = node.result[0];

	res.reservationNumber = pass.barcodes[0].alternativeText;
	res.reservedTicket.name = pass.field['tickets'].value;
	res.reservationFor.startTime = pass.field['date_time'].value
	res.reservationFor.endTime = parseDurationAsDate(res.reservationFor.startTime, pass.field['duration'].value);
	
	return res
}

function parseDurationAsDate(startTime, duration) {
	let setDate = new Date(startTime);
	let [ _, hours, minutes ] = duration.match(/^(?:(\d+) hours?)?\s?(?:(\d+) minutes?)?$/);
	setDate.setMinutes(setDate.getMinutes() + (parseInt(minutes) || 0));
	setDate.setHours(setDate.getHours() + (parseInt(hours) || 0));
	return setDate;
}

function extractPdf(pdf, node, barcode) {
	const res = JsonLd.newEventReservation();
	const page = pdf.pages[barcode.location]
	const text = page.text

	// in example: "Date: Tue 19 Aug, 2025"
	let date = text.match(/Date:\s*(.*)\s+/);
	// in example: "Time of visit: 06:00 PM"
	let time = text.match(/Time of visit:\s*(.*)\s+/);
	res.reservationFor.startDate = JsonLd.toDateTime(date[1] + ' ' + time[1], "ddd d MMM, yyyy hh:mm a", ["en", "nl"]);

	// in example: "Ticket: Complete Entry"
	res.reservationFor.name = text.match(/Ticket:\s*(.*)\s+/)[1];

	// in example: "Guest: Guest: 2x Adult (12+)"
	let ticketType = text.match(/Guest:\s*(.*)\s+/)[1];
	res.reservedTicket.name = ticketType

	// in example: "Booking Reference: XXXXXXXX"
	res.reservationNumber = text.match(/Booking Reference:\s*(.*)\s+/)[1];

	res.underName.name = text.match(/Name:\s*(.*)\s+/)[1];
	res.reservedTicket.ticketToken = 'qrCode:' + (barcode.content ?? res.reservationNumber);

	// Sometimes the price appears
	ExtractorEngine.extractPrice(page.text, res);

	return res
}