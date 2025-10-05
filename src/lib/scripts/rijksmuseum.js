// SPDX-FileCopyrightText: 2025 David Pilarčík <meow@charliecat.space>
// SPDX-License-Identifier: LGPL-2.0-or-later

const i18n = {
	"en": {
		__lookingFor: "TYPE OF TICKET",
		"Ticket type": /TYPE OF TICKET \s+ (.*)/,
		"Starting time between": /STARTING TIME BETWEEN \s+ (\d{2}\/\d{2}\/\d{4} \d{1,2}:\d{2}) and (\d{2}\/\d{2}\/\d{4} \d{1,2}:\d{2})/,
		"Under Name": /NAME \s+ (.*)/,
		"Price": /PRICE \s+ (\d+.\d+)/
	},
	"nl": {
		__lookingFor: "TICKETSOORT",
		"Ticket type": /TICKETSOORT \s+ (.*)/,
		"Starting time between": /STARTTIJD TUSSEN \s+ (\d{2}\/\d{2}\/\d{4} \d{1,2}:\d{2}) en (\d{2}\/\d{2}\/\d{4} \d{1,2}:\d{2})/,
		"Under Name": /NAAM \s+ (.*)/,
		"Price": /PRIJS \s+ (\d+.\d+)/
	}
}

function parsePdf(pdf, node, triggerNode) {
	const page = pdf.pages[triggerNode.location];
	const res = JsonLd.newEventReservation();

	res.reservedTicket.ticketToken = 'code128:' + triggerNode.content;
	res.reservationNumber = triggerNode.content;

	let lang = i18n.en;
	for (let language in i18n) {
		if (page.text.includes(i18n[language].__lookingFor)) lang = i18n[language];
	}

	res.reservedTicket.name = lang["Ticket type"].exec(page.text)?.[1] || null;

	// Ticket has no specific name
	res.reservationFor.name = `Rijksmuseum - ${res.reservedTicket.name}`;

	// in example "16/08/2025 11:00"
	// ticket books a time slot, in which the visitor can enter the museum
	// as per the ticket:
	// > You have a ticket indicating a start time. This means
	// > that you can begin your visit to the Rijksmuseum
	// > within this time frame.
	// > Once you have begun your visit, you may stay as
	// > long as you wish.
	let [_t, startingTime, slotEndingTime] = lang["Starting time between"].exec(page.text) ?? ["", "", ""]
	res.reservationFor.startDate = JsonLd.toDateTime(startingTime, "dd/MM/yyyy h:mm", ["en", "nl"])
	res.reservationFor.doorsClose = JsonLd.toDateTime(slotEndingTime, "dd/MM/yyyy h:mm", ["en", "nl"])

	res.underName.name = lang["Under Name"].exec(page.text)?.[1] || null;

	// Ticket has price listed, but no specific currency
	// From the Netherlands having the Euro, the currency is set to EUR
	res.totalPrice = parseInt(lang["Price"].exec(page.text)?.[1] || 0);
	res.priceCurrency = "EUR";

	// Assuming that Rijksmuseum won't sell tickets outside it's own venue
	res.reservationFor.location.name = "Rijksmuseum"
	res.reservationFor.location.address.streetAddress = "Museumstraat 1"
	res.reservationFor.location.address.addressLocality = "Amsterdam"
	res.reservationFor.location.address.postalCode = "1071 XX"
	res.reservationFor.location.address.addressCountry = "NL"

	return res
}