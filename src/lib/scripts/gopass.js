// SPDX-FileCopyrightText: 2025 David Pilarcik <meow@charliecat.space>
// SPDX-License-Identifier: LGPL-2.0-or-later

const i18n = {
	'sk': {
		__lookingFor: 'Číslo karty',
		'Confirmation of purchase': 'Potvrdenie o nákupe',
		'Card number': 'Číslo karty',
		'Ticket Number': 'Číslo lístku',
		'QR Code': 'QR kód',
		'You can pass through the turnstile by scanning the QR code.': 'Turniketom prejdete načítaním QR kódu.'
	}
}

function parsePdfConfirmation(pdf, node, triggerNode) {
	const page = pdf.pages[triggerNode.location];
	const text = page.text;
	let res = JsonLd.newEventReservation();

	let lang = i18n['sk'];
	for (let l in i18n) {
		if (text.includes(i18n[l].__lookingFor)) {
			lang = i18n[l];
			break;
		}
	}

	// the "Potvrdenie o nákupe" string shows up in the header of the first page
	let headerOffset = page.text.includes(lang['Confirmation of purchase']) ? 0.06 : 0;
	// Example: "Jan Novák\nŠtudent"
	let [holderName, holderAgeGroup] = page.textInRect(0, headerOffset+0.11, 0.3, 0.25).split('\n');
	res.underName.name = holderName.trim();
	res.reservedTicket.name = holderAgeGroup.trim();

	// For difffrent header sizes have to adjust the rectangle
	res.reservationFor.name = page.textInRect(0, headerOffset, 0.5, 0.15).split(holderName)[0];

	// Example: "Číslo lístku\n8f907e3b1928172c6e08baee36"
	// Ticket number is diffrent pattern is diffrent for venue types.
	// Tatralandia uses UUID-like ticker numbers
	// Others use tickers like: "1-1760-66-205420"
	res.reservationNumber = text.match(new RegExp(lang['Ticket Number'] + '\\s+(.*)\\s+'))[1];

	let date = page.textInRect(0.6, headerOffset, 1, 0.2).split('\n')[0].trim();
	// Example: "23. 7. 2025" or "24. 3. 2023 - 27. 3. 2023"
	if (date.includes(" - ")) {
		let [startDate, endDate] = date.split(" - ");
		res.reservationFor.startDate = JsonLd.toDateTime(startDate, 'd. M. yyyy', 'sk');
		res.reservationFor.endDate = JsonLd.toDateTime(endDate, 'd. M. yyyy', 'sk');
	} else {
		res.reservationFor.startDate = JsonLd.toDateTime(date, 'd. M. yyyy', 'sk');
	}

	if (page.text.includes("PIN:")) {
		// Extract the PIN from the text
		// Ticket for establishments like Tatralandia or Besenova allow self-check-in with PIN from the ticket
		// other tickets do not have a PIN. user checks in with their GoPass Card Code or QR code if gived
		// Example: "PIN: 123456"
		res.reservationFor.description = /PIN: (\d+) /.exec(page.text)?.[1];
		res.reservedTicket.ticketNumber = res.description;
	}

	if (page.text.includes(lang['QR Code'])) {
		// Looking through the page for a QR code
		// QR code never is the ticket number nor the GoPass Card Code
		let qrCodeApplicable = node.childNodes
			.filter(c => c.mimeType == 'internal/qimage')
			.map(c => c.childNodes[0]?.content)
			.filter(c => c != undefined && c.length >= 24 && c !== res.reservationNumber);
		if (qrCodeApplicable.length != 0) res.reservedTicket.ticketToken = 'qrcode:' + qrCodeApplicable[triggerNode.location];
	} else if (page.text.includes(lang['You can pass through the turnstile by scanning the QR code.'])) {
		// If the ticket says to scan the QR code at the turnstile,
		// and the ticket doesn't have a QR code,
		// we can assume the QR code is the ticket number in a barcode format.
		res.reservedTicket.ticketToken = 'code128:' + res.reservationNumber;
	} else {
		// At the end, we can assume the ticket, if needed to be scanned, is the the same code as GoPass Card Code
		res.reservedTicket.ticketToken = 'code128:' + triggerNode.content;
	}

	// The Membership number is the GoPass Card Number
	res.programMembershipUsed = { membershipNumber: triggerNode.content, programName: "Gopass" };

	ExtractorEngine.extractPrice(res.reservedTicket, text);

	// console.log(JSON.stringify(res, null, 4));
	// Example: "Využitie na miestach:\n• Vodný park Tatralandia"
	// Gopass allows for tickets to be used at multiple places.
	// The places are listed in the ticket.
	// Reason why? Gopass allows purchase of multiday skipasses for ski resorts,
	// which use the same ticket PDF template
	let placesApplicable = page.textInRect(0.3, headerOffset+0.11, 0.7, 0.4)
		.split('\n')
		.filter(line => line.includes('• '))
		.map(line => line.replace('• ', '').trim());
	res.reservationFor.location.name = placesApplicable.join(', ');

	return res;
}