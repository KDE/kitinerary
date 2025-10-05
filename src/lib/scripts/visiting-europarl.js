// SPDX-FileCopyrightText: 2025 David Pilarčík <meow@charliecat.space>
// SPDX-License-Identifier: LGPL-2.0-or-later

function parsePdf(pdf, node, triggerNode) {
    const page = pdf.pages[triggerNode.location];
	const res = JsonLd.newEventReservation();

	res.reservedTicket.ticketToken = 'qrCode:' + triggerNode.content;
	res.reservationNumber = page.text.match(/[0-9a-f]{13}/)[0];

	// in example: "11:00 Monday, August 25, 2025"
	let dateAndTime = page.textInRect(0, 0.2, 0.5, 0.25).split("\n").join(" ").trim();
	res.reservationFor.startDate = JsonLd.toDateTime(dateAndTime, "hh:mm dddd, MMMM d, yyyy", "en")
	
	let eventNameAndUnderName = page.textInRect(0, 0.25, 0.7, 0.3).split("\n");
	res.reservationFor.name = eventNameAndUnderName[0];
	// in example: "Created by David Pilarčík on 28.06.2025 17:47"
	let [_u, name, creationDate] = /Created by (.*?) on (.*?)/.exec(eventNameAndUnderName[1]);
	res.underName.name = name;
	// in example: "28.06.2025 17:47"
	res.bookingTime = JsonLd.toDateTime(creationDate, "dd.MM.yyyy hh:mm", "en");

	// in exmaple:
	// European Parliament
	// Rue Wiertz/Wiertzstraat 60
	// 1047 Bruxelles/Brussel
	// BELGIUM
	let [venue, streetAddress, locality, country] = page.textInRect(0, 0.9, 0.3, 1).split("\n");
	res.reservationFor.location.name = venue;
	res.reservationFor.location.address.streetAddress = streetAddress;
	res.reservationFor.location.address.addressLocality = locality;
	res.reservationFor.location.address.addressCountry = country;

	// Given tour/visit confirmation includes attendee information
	// These are reviewed by in person when checking in into the venue
	// Usually by an official identity document
	if (page.text.includes("Name      ")) {
		res.reservationFor.attendees = []
		let attendeesText = page.textInRect(0, 0.3, 1, 0.6).split("\n");
		for (let attendee of attendeesText) {
			if (attendee.startsWith("*")) break;
			if (attendee.startsWith("Name")) continue;
			// in example: "John Doe    AB123456      01.01.1990"
			let [_a, name, passportNumber, dob] = /^(.*)\s+([A-Z]{2}\d+)\s+(\d{2}\.\d{2}\.\d{4})$/.exec(attendee);
			res.reservationFor.attendees.push({
				"@type": "Person",
				name: name,
				passportNumber: passportNumber,
				dateOfBirth: JsonLd.toDateTime(dob, "dd.MM.yyyy", "en")
			});
		}
	}

	return res;
}