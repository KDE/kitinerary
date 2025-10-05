// SPDX-FileCopyrightText: 2025 David Pilarčík <meow@charliecat.space>
// SPDX-License-Identifier: LGPL-2.0-or-later

function parsePdf(pdf, node, triggerNode) {
    const page = pdf.pages[triggerNode.location];
	const res = JsonLd.newEventReservation();

	res.reservedTicket.ticketToken = 'qrCode:' + triggerNode.content;

	// in example:
	// Večer Zvedavých
	// Černyševského 3761/42
	// 20. november 2024 19:00
	let [eventName, venueName, date] = page.textInRect(0.1, 0, 0.6, 0.08).split("\n")
	res.reservationFor.name = eventName;
	res.reservationFor.location.name = venueName;
	res.reservationFor.startDate = JsonLd.toDateTime(
		date, 
		["d. MMMM yyyy h:mm", "d.M.yyyy h:mm:ss", "d.M.yyyy h:mm"],  
		["sk", "en"] // not all Slovak Month names are parsable
	);

	res.reservationNumber = page.text.match(/ID: (\d+)/)[1];

	let [_o, orgName, streetAddress, addressLocality, postalCode] = /Organizátor: (.*), (.*), (.*), (\d{3} \d{2}), (?:IČO: .*, )?(?:DIČ: .*, )?(?:IČ DPH: [A-Z]{2}\d+)?/g.exec(page.text);
	res.provider = {
		"@type": "Organization",
		name: orgName,
		location: {
			"@type": "Place",
			address: {
				"@type": "PostalAddress",
				streetAddress,
				addressLocality,
				postalCode
			}
		}
	}

	// in example: "Dátum a čas predaja: 02.11.2024 12:54"
		res.bookingTime = JsonLd.toDateTime(
			page.text.match(/Dátum a čas predaja: (.+)/)?.[1] ?? "", 
			["dd. MM. yyyy hh:mm", "dd.MM.yyyy hh:mm"], 
			"sk"
		);

	// this is optional, not every ticket includes this
	let underName = page.textInRect(0.6, 0.04, 1, 0.1)
	if (underName.length != 0) res.underName.name = page.textInRect(0.6, 0, 1, 0.1)

	// some events allow for seat selection
	if (page.text.includes("Miesto:")) {
		let ticketText = page.textInRect(0.1, 0, 1, 1).split("\n").map(e=> e.trim()).filter(e => e.length > 0);
		let seatInfo = ticketText.find(e => e.includes("Miesto:") || e.includes("Sektor:") || e.includes("Rad:"));
		// from examples:
		// Gate: B, Sektor: 24, Rad: 1, Miesto: 2
		// Prízemie I., Rad: XIV., Miesto: 8
		// Sektor SKYBOX, Rad: 14, Miesto: 1
		// Rad: 5, Miesto: 10
		// From the looks of it, Sector is Venue defined text
		// Hope there isnt a sample with only the seat without a row
		// Gate in this sense is the asked fastest enterace of the venue, to the seat
		// Can't extract this, as the `ticketedSeat` property doesn't know such thing
		
		let [_, sector, row, seat] = /(?:(.*), )?Rad: (.*), Miesto: (.*)/.exec(seatInfo);
		res.reservedTicket.ticketedSeat = {
			"@type": "Seat",
			seatNumber: seat,
			seatRow: row,
			seatSection: sector
		};
	}

	ExtractorEngine.extractPrice(page.text, res);

    return res;
}