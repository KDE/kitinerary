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