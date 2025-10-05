// SPDX-FileCopyrightText: 2025 David Pilarčík <meow@charliecat.space>
// SPDX-License-Identifier: LGPL-2.0-or-later

const i18n = {
	"nl-NL": {
		__lookingFor: "Geldig op:",
		"under name": /Naam: \s+ (.*)/,
		"ticket type": /Tickettype: \s+ (.*)/,
		"valid at": /Geldig op: \s+ (.*)/,
		"event": /Evenement: \s+ (.*)/
	}
}

function parsePkPass(pass, node) {
	const res = Object.assign(JsonLd.newEventReservation(), node.result[0]);

	if (res.reservationFor.name == "Passbook ticket") {
		res.reservationFor.name = pass.organizationName
	}

	res.reservationFor.startDate = JsonLd.toDateTime(
		pass.field['ucRow3Field1Label'].value + " " + pass.field['ucRow3Field2Label'].value, 
		"dd-MM-yyyy hh:mm", 
		[ "en", "nl" ]
	) // 16-08-2025 16:30

	res.reservationNumber = pass.barcodes[0].alternativeText

	res.reservedTicket.name = pass.field["ucRow2Field1Label"].value

	return res
}

function parsePdf(pdf, node, triggerNode) {
    const page = pdf.pages[triggerNode.location];
	const res = JsonLd.newEventReservation();

	res.reservedTicket.ticketToken = 'qrCode:' + triggerNode.content;
	res.reservationNumber = triggerNode.content // Ticket token is also shown in the text contents of the ticket

	let locale = i18n["nl-NL"];
	for (let language in i18n) {
		if (page.text.includes(language.__lookingFor)) {
			locale = language;
			break;
		}
	}

	res.underName.name = locale["under name"].exec(page.text)[1];
	res.reservedTicket.ticketType = locale["ticket type"].exec(page.text)[1];
	// educated guess
	res.reservationFor.name = (locale["event"].exec(page.text) ?? ["", "Event"])[1];
	
	let startDate = locale["valid at"].exec(page.text)[1];
	// in example: "16-08-2025 16:30"
	res.reservationFor.startDate = JsonLd.toDateTime(startDate, "dd-MM-yyyy hh:mm", [ "en", "nl" ]);

	return res
}