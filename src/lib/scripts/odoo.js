// SPDX-FileCopyrightText: 2025 David Pilarčík <meow@charliecat.space>
// SPDX-License-Identifier: LGPL-2.0-or-later

function parsePdf(pdf, node, triggerNode) {
	let res = JsonLd.newEventReservation();
	const page = pdf.pages[triggerNode.location];
	res.reservedTicket.ticketToken = "qrCode:" + triggerNode.content
	res.reservationFor.name = page.textInRect(0.0, 0.0, 1.0, 0.1)
	
	let timeDetails = page.textInRect(0.5, 0.25, 1.0, 0.5)
	let date = timeDetails.match(/\d+\/\d+\/\d+/)[0]; // in sample "05/22/2025"
	let [_t, timeFrom, timeTo] = /from (\d+:\d+ [AP]M) to (\d+:\d+ [AP]M)/g.exec(timeDetails); // in sample "from 6:00 PM to 9:00 PM"
	res.reservationFor.startDate = JsonLd.toDateTime(date + " " + timeFrom, "MM/dd/yyyy h:mm a", "en");
	res.reservationFor.endDate = JsonLd.toDateTime(date + " " + timeTo, "MM/dd/yyyy h:mm a", "en");

	// in sample 
	// "Hotel Devin Bratislava
	//	Riečna ulica 162/4, 811 02
	//	Bratislava
	//	Slovakia"
	let locationDetails = page.textInRect(0, 0.25, 0.5, 0.5).split("\n");
	res.reservationFor.location.name = locationDetails[0].replace("", "");
	res.reservationFor.location.address.streetAddress = locationDetails[1];
	res.reservationFor.location.address.addressLocality = locationDetails[2];
	res.reservationFor.location.address.addressCountry = locationDetails[3];

	// in sample 
	// "<event name>
	//	Registration for <event name>
	//	John Doe"
	res.underName.name = page.textInRect(0.0, 0.1, 1.0, 0.2).trim().split("\n").filter(line => !line.includes("Registration for")).pop();

	// in sample "Odoo S.A.  +32 81 81 37 00  info@odoo.com  https://www.odoo.com"
	let oraganizerDetails = page.textInRect(0.0, 0.9, 1, 1).split(/(?= [])/).map(part => part.trim())
	res.provider = {
		"@type": "Organization",
		name: oraganizerDetails[0],
		telephone: oraganizerDetails.find(part => part.startsWith("")).replace("", "").trim(),
		email: oraganizerDetails.find(part => part.startsWith("")).replace("", "").trim(),
		url: oraganizerDetails.find(part => part.startsWith("")).replace("", "").trim()
	}

	return res;
}