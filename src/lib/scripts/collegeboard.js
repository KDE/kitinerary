// SPDX-FileCopyrightText: 2026 David Pilarčík <meow@charliecat.space>
// SPDX-License-Identifier: LGPL-2.0-or-later

function extractHtml(html, node) {
	const text = String(html.root.recursiveContent);
	if (!text.includes("Your SAT Admission Ticket")) return [];
	const res = JsonLd.newEventReservation();
	
	res.underName.name = /Name: (.*)/.exec(text)[1]
	res.reservationNumber = /Registration Number\n(\d+)\n/.exec(text)[1]

	let date = /Date\n[A-z]{3}, ([A-z]{3} \d{1,2}, \d{4})/.exec(text)[1] // Mar 14, 2026
	// 7:45 a.m. CET => 7:45 AM CET
	let arrivalTime = /Arrival time\n(\d+:\d+ .*)\n/.exec(text)[1].replace("a.m.", "AM").replace("p.m.", "PM")
	let doorsCloseTime = /Doors Close\n(\d+:\d+ .*)\n/.exec(text)[1].replace("a.m.", "AM").replace("p.m.", "PM")
	res.reservationFor.startDate = JsonLd.toDateTime(
		`${date} ${arrivalTime}`,
		"MMM d, yyyy h:mm AP t", /* example: "Mar 14, 2026 7:45 AM CET" */
		"en"
	)
	res.reservationFor.doorsClose = JsonLd.toDateTime(
		`${date} ${doorsCloseTime}`,
		"MMM d, yyyy h:mm AP t",
		"en"
	)

	// Unsure how they send testing centre tickets for other types of tests
	res.reservationFor.name = "Your SAT Admission Ticket" // doesnt have a name tbh
	
	// example:
	// BRITISH INTL SCH BRATISLAVA
	// PEKNIKOVA 6
	// BRATISLAVA,
	// SK
	let location = /Location\n(.*)\n(.*)\n(.*)\n(.*)\n/.exec(text)
	res.reservationFor.location.name = location[1]
	res.reservationFor.location.address.streetAddress = location[2]
	res.reservationFor.location.address.addressLocality = location[3]
	res.reservationFor.location.address.addressCountry = location[4]
	
	// Ticket Token is not present in the HTML, but we can construct it from the reservation number
	// It only appears on the testing device screen (after going through the "Device Check In") and the Printed Paper Ticket, but in the email.
	res.reservedTicket.ticketToken = "qrCode:" + res.reservationNumber;
	
	return res
}