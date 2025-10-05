// SPDX-FileCopyrightText: 2025 David Pilarčík <meow@charliecat.space>
// SPDX-License-Identifier: LGPL-2.0-or-later

function parsePkPass(pass, node) {
    const res = node.result[0];

	res.reservationNumber = pass.field['ticketnummer'].value
	// in example: "Saturday 16 August 2025, 16:00"
	res.reservationFor.startDate = JsonLd.toDateTime(pass.field['date'].value, "dddd d MMMM yyyy, HH:mm", ["en", "nl"])
	res.provider = {
		"@type": "Organization",
		"name": pass.organizationName,
	}
	// in example: { "key": "event", "label": "Type ticket", "value": "Student" }
	if (pass.field['event'].label.includes("Type")) {
		res.reservedTicket.name = pass.field['event'].value
	} else {
		res.reservationFor.name = pass.field['event'].value
	}

	// in example: 
	// <Street Address>\n<Postal Code & City>\n<Phone Number>\n<Web site>
	let contactAsArray = pass.field['contact'].value?.split('\n');
	if (pass.field['contact'].value) {
		res.reservationFor.location = JsonLd.newObject("Place");
		res.reservationFor.location.address = JsonLd.newObject("PostalAddress");
		res.reservationFor.location.address.streetAddress = contactAsArray[0];
		res.reservationFor.location.address.addressLocality = contactAsArray[1];
		res.provider.telephone = contactAsArray[2];
		res.provider.url = contactAsArray[3];
	}

	return res
}

function extractPdf(pdf, node, barcode) {
	const res = JsonLd.newEventReservation();
	const page = pdf.pages[barcode.location]
	const details = page.textInRect(0.7, 0, 1, 1);

	// Ussually E-Ticket
	res.reservationFor.name = details.split("\n")[0]

	// In example: "Ticket number\n77110300000001000"
	res.reservationNumber = details.match(/Ticket number\s+(\d+)/)[1] ?? barcode.content;
	res.reservedTicket.ticketToken = 'qrCode:' + barcode.content;
	// In example: "Ticket type\nStudent"
	res.reservedTicket.name = details.match(/Ticket type\s+(.+)\s+/)[1];
	// In example: "Visitor\nDavid Pilarcik"
	res.underName.name = details.match(/Visitor\s+(.+)\s+/)[1];

	// In example: "Valid on\nSaturday 16 August 2025, 16:00"
	res.reservationFor.startDate = JsonLd.toDateTime(details.match(/Valid on\s+(.+)\s+/)[1], "dddd d MMMM yyyy, HH:mm", ["en", "nl"]);
	// In example: "Date of purchase\n20-07-2025"
	res.bookingTime = JsonLd.toDateTime(details.match(/Date of purchase\s+(.+)\s+/)[1], "dd-MM-yyyy", ["en", "nl"]);

	// in example: 
	// <Street Address>\n<Postal Code & City>\n<Phone Number>\n<Web site>
	let contactAsArray = details.match(/Contact\s+(.+)\s+(.+)\s+(.+)\n(.+)/);
	res.reservationFor.location = JsonLd.newObject("Place");
	res.reservationFor.location.address = JsonLd.newObject("PostalAddress");
	res.reservationFor.location.address.streetAddress = contactAsArray[1];
	res.reservationFor.location.address.addressLocality = contactAsArray[2];
	res.provider = JsonLd.newObject("Organization");
	res.provider.telephone = !isNaN(parseInt(contactAsArray[3])) ? contactAsArray[3] : undefined;
	res.provider.url = contactAsArray[4].match(/\S+\.\S+/) ? contactAsArray[4] : undefined;

	// Each venue sets their own description. Could be the exposition or the event description itself or ticket rules
	res.reservationFor.description = page.textInRect(0, 0, 0.7, 0.9);

	// Sometimes the price appears
	ExtractorEngine.extractPrice(page.text, res);

	return res
}