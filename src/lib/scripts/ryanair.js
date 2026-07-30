/*
   SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
   SPDX-FileCopyrightText: 2026 David Pilarčík <meow@charliecat.space>
   SPDX-License-Identifier: LGPL-2.0-or-later
*/

const IATA_CODES = {
	"FR": "Ryanair DAC",
	"RR": "Buzz (Ryanair Sun)",
	"AL": "Malta Air",
	"OE": "Lauda Europe",
	"RK": "Ryanair UK"	
}

function parseBoardingPass(pdf, node, triggerNode) {
    const page = pdf.pages[triggerNode.location];
    let res = triggerNode.result[0];

    const topRight = page.textInRect(0.5, 0.0, 1.0, 0.5);
    const depTimes = topRight.match(/(\d\d:\d\d).*(\d\d:\d\d)/);
    res.reservationFor.boardingTime = JsonLd.toDateTime(depTimes[1], "hh:mm", "en");
    res.reservationFor.departureTime = JsonLd.toDateTime(depTimes[2], "hh:mm", "en");

    const bottomRight = page.textInRect(0.0, 0.0, 0.5, 0.5, 180);
    const times = bottomRight.match(/(\d\d:\d\d)/g);
    if (times.length > 3 && times[times.length - 1] != depTimes[2])
        res.reservationFor.arrivalTime = JsonLd.toDateTime(times[times.length -1], "hh:mm", "en");

    return res;
}

function fixHtmlBooking(html, node) {
    let reservations = node.result;
    let iataCodeMap = {};
    // fix missing IATA codes in arrival airports on return trips
    for (r of reservations)
        iataCodeMap[r.reservationFor.departureAirport.name] = r.reservationFor.departureAirport.iataCode;
    for (r of reservations) {
        if (r.reservationFor.arrivalAirport.iataCode === "") {
            r.reservationFor.arrivalAirport.iataCode = iataCodeMap[r.reservationFor.arrivalAirport.name];
        }
    }

    return reservations;
}

function enchanceFromBCBP(bcbp, node) {
	const result = node.result;

	for (let i in result) {
		// Ryanair send a link to the Itinerary in the reservation confirmation
		// this link allows to view and edit (after logging in) the Itinerary
		let res = result[i]
		result[i].potentialAction = JsonLd.newObject("UpdateAction");
		result[i].potentialAction.url = `https://www.ryanair.com/ie/en/trip/manage/PNR:${res.reservationNumber}`
		result[i].reservationFor.airline.name = IATA_CODES[res.reservationFor.airline.iataCode] || res.reservationFor.airline.name;
	}
	
	return result
}

function extractPKPass(pass, node) {
	const res = node.result[0];
	res.boardingGroup = `${pass.field['queue'].value} + ${pass.field["door"].value}`
	res.reservationFor.departureTime = res.reservationFor.boardingTime
	res.reservationFor.boardingTime = null;
	res.freeBaggageAllowance = pass.field['cabin_bags'].value;
	res.reservationFor.departureGateCloseTime = pass.field["gateClose"].value
	// might be more than just preorded meal. usually just preorded meal
	res.mealService = pass.field['ssr'].value; 
	return res
}
