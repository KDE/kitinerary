/*
   SPDX-FileCopyrightText: ⓒ 2026 David Pilarcik <meow@charliecat.space>
   SPDX-License-Identifier: LGPL-2.0-or-later
*/

const WIZZ_AIR_IATA_CODES = {
	"W6": "Wizz Air Hungary",
	"W4": "Wizz Air Malta",
	"W9": "Wizz Air UK",
	"5W": "Wizz Air Abu Dhabi"
};

function enchanceFromBCBP(bcbp, node) {
	const result = node.result;

	for (const res of result) {
		// Wizz air allows Passanger to share a link from their mobile
		// this link allows to view and edit (after logging in) the Itinerary
		res.potentialAction = JsonLd.newObject("UpdateAction");
		res.potentialAction.url = `https://www.wizzair.com/en-gb/itinerary/${res.reservationNumber}/${res.underName.familyName}`
		res.reservationFor.airline.name = WIZZ_AIR_IATA_CODES[res.reservationFor.airline.iataCode] || res.reservationFor.airline.name;
	}
	
	return result
}

function extractPKPass(pass, node) {
	const result = node.result;

	for (const res of result) {
		// Wizz Air mints PkPasses with the 'depart' and 'arrive' field label's including Terminal information if the Airport has multiple operating terminals
		// 29. December 2025 - onward
		// "Bratislava"
		// "Kosice" - has only one terminal, so no terminal information is included in the label
		// "Terminal 1 - Vienna" - multiple terminals
		// "Terminal 2 - Barcelona El Prat" - multiple terminals
		// 
		// 2021 Boarding passes show (only 1 sample)
		// "Vienna - Terminal 1"
		if (res.reservationFor.departureAirport.name.startsWith("Terminal ")) {
			let [_, terminal, airportName] = /Terminal (.*) - (.*)/.exec(res.reservationFor.departureAirport.name);
			res.reservationFor.departureAirport.name = airportName;
			res.reservationFor.departureTerminal = terminal;
		}
		if (res.reservationFor.arrivalAirport.name.startsWith("Terminal ")) {
			let [_, terminal, airportName] = /Terminal (.*) - (.*)/.exec(res.reservationFor.arrivalAirport.name);
			res.reservationFor.arrivalAirport.name = airportName;
			res.reservationFor.arrivalTerminal = terminal;
		}

		// value is either "Yes" or "No"
		if (pass.field['priority'].value == "Yes") res.boardingGroup = "Wizz Priority";

		// e.g. "Free under-seat bag" or "Free carry-on bag"
		res.freeBaggageAllowance = pass.field['baggage'].value;

		// Check if this field exists?
		let departureGateCloseTime = new Date(pass.relevantDate);
		// "05:30"
		let [gateclosesHours, gateclosesMinutes] = pass.field['gatecloses'].value.split(':').map(x => parseInt(x));
		departureGateCloseTime.setHours(gateclosesHours);
		departureGateCloseTime.setMinutes(gateclosesMinutes);
		res.reservationFor.departureGateCloseTime = departureGateCloseTime
	}

	return result
}

function parseMail(html) {
	const result = []
	const baseRes = JsonLd.newFlightReservation();

	baseRes.reservationNumber = html.eval("//td[contains(normalize-space(.),'Flight confirmation code:')]/following-sibling::td[1]")[0].content.trim();
	let [_fn, iataCode, flightNumber] = 
		// Flight Number: W6 7001
		/Flight Number: (.{2}) (\d+)/.exec(html.eval("//tr[td[contains(normalize-space(.),'GOING OUT')]]/td[3]")[0].content.trim());
	baseRes.reservationFor.airline.iataCode = iataCode;
	baseRes.reservationFor.airline.name = WIZZ_AIR_IATA_CODES[iataCode] || baseRes.reservationFor.airline.name;
	baseRes.reservationFor.flightNumber = flightNumber;

	let [_d, departingCity, departingAirportIATACode] =
		// Bratislava (BTS)
		/(.*) \((.{3})\)/.exec(html.eval("//tr[td[contains(normalize-space(.),'Departs from:')]]/following-sibling::tr[1]/td[1]")[0].content.trim());
	let [_a, arrivingCity, arrivingAirportIATACode] =
		// Kosice (KSC)
		/(.*) \((.{3})\)/.exec(html.eval("//tr[td[contains(normalize-space(.),'Departs from:')]]/following-sibling::tr[1]/td[3]")[0].content.trim());

	baseRes.reservationFor.departureAirport.name = departingCity;
	baseRes.reservationFor.departureAirport.iataCode = departingAirportIATACode;
	baseRes.reservationFor.arrivalAirport.name = arrivingCity;
	baseRes.reservationFor.arrivalAirport.iataCode = arrivingAirportIATACode;

	baseRes.reservationFor.departureTime = JsonLd.toDateTime(
		// 22/07/2026 06:00
		html.eval("//tr[td[contains(normalize-space(.),'Departs from:')]]/following-sibling::tr[2]/td[1]")[0].content.trim(),
		"dd/MM/yyyy hh:mm",
		"en"
	);
	baseRes.reservationFor.departureTime = JsonLd.toDateTime(
		// 22/07/2026 06:55
		html.eval("//tr[td[contains(normalize-space(.),'Departs from:')]]/following-sibling::tr[2]/td[3]")[0].content.trim(),
		"dd/MM/yyyy hh:mm",
		"en"
	);

	ExtractorEngine.extractPrice(html.eval("//tr[td[normalize-space(.)='Grand total']]")[0].recursiveContent.split("\n").join(""), baseRes);

	let passangers = html.eval("//tr[td[normalize-space(.)='Title'] and td[normalize-space(.)='First name']]/following-sibling::tr[position()>1]")
	for (const passangerRow of passangers) {
		let res = Object.assign({}, baseRes);
		let passanger = passangerRow.eval("td").map(e => e.content.trim());
		res.underName.familyName = passanger[2]
		res.underName.givenName = passanger[1]
		res.underName.title = passanger[0]
		res.airplaneSeat = passanger[6]

		res.potentialAction = JsonLd.newObject("UpdateAction");
		res.potentialAction.url = `https://www.wizzair.com/en-gb/itinerary/${res.reservationNumber}/${res.underName.familyName}`

		result.push(res);
	}

	return result
}