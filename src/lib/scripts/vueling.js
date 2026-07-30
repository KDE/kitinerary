/*
   SPDX-FileCopyrightText: 2017 Volker Krause <vkrause@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

function parseHtmlBooking(doc) {
    var reservations = new Array();

    var bookingRef = doc.eval("//td[@class=\"vuelo_confirmado_header\"]")[0].firstChild.content;
    if (!bookingRef)
        return null;

    var lang = doc.eval('//a[@class="copyright-text"]')[0].attribute("href").match(/\/([a-z]{2})$/)[1];

    var elems = doc.eval("//td[@class=\"vuelo_confirmado_card__subheader\"]");
    for (var i = 0; i < elems.length; ++i) {
        var elem = elems[i];
        var detailsRoot = elem.parent.nextSibling;

        var res = JsonLd.newFlightReservation();
        res.reservationNumber = bookingRef;

        var airportName = detailsRoot.eval(".//td[@class=\"vuelo_confirmado_card_details--city salida\"]")[0];
        var iataCode = detailsRoot.eval(".//td[@class=\"vuelo_confirmado_card_details--iata\"]")[0];
        res.reservationFor.departureAirport.iataCode = iataCode.content;
        res.reservationFor.departureAirport.name = airportName.content;
        res.reservationFor.arrivalAirport.iataCode = iataCode.nextSibling.content;
        res.reservationFor.arrivalAirport.name = airportName.nextSibling.content;

        var time = detailsRoot.eval(".//td[@class=\"vuelo_confirmado_card_details--time\"]")[0];
        res.reservationFor.departureTime = JsonLd.toDateTime(elem.content + ' ' + time.content.replace('h', ''), "dddd, dd MMMM yyyy HH:mm", lang);
        res.reservationFor.arrivalTime = JsonLd.toDateTime(elem.content + ' ' + time.nextSibling.content.replace('h', ''), "dddd, dd MMMM yyyy HH:mm", lang);

        var flightNum = detailsRoot.recursiveContent.match(/\s([A-Z0-9]{2})(\d{1,4})\b/);
        res.reservationFor.flightNumber = flightNum[2];
        res.reservationFor.airline.iataCode = flightNum[1];

        reservations.push(res);
    }

    return reservations;
}

function parsePdfBoardingPass(pdf, node, triggerNode) {
    let res = triggerNode.result[0];
    const page = pdf.pages[triggerNode.location];
    const topRight = page.textInRect(0.5, 0.0, 1.0, 0.5);
    const times = topRight.match(/(\d{2}:\d{2}) (?:H|Uhr) +(\d{2}:\d{2}) (?:H|Uhr)/);
    res.reservationFor.departureTime = JsonLd.toDateTime(times[1], "hh:mm", "en");
    res.reservationFor.arrivalTime = JsonLd.toDateTime(times[2], "hh:mm", "en");
    const topLeft = page.textInRect(0.0, 0.0, 0.5, 1.0);
    const grp = topLeft.match(/(?:Group|Grupo|Gruppe) +(\d)/i);
    if (grp) {
        res.boardingGroup = grp[1];
    }
    return res;
}

function enchanceFromBCBP(bcbp, node) {
	const result = node.result;
	for (const res of result) {
		// Vueling send a link to the Itinerary in the reservation confirmation
		// this link allows to view and edit (after logging in) the Itinerary
		// user has to be on mobile for this
		res.potentialAction = JsonLd.newObject("UpdateAction");
		// https://mobile.vueling.com/MyBookings/Index?pnr=[PNR]&o=[departure airport iata code]
		res.potentialAction.url = `https://mobile.vueling.com/MyBookings/Index?pnr=${res.reservationNumber}&o=${res.reservationFor.departureAirport.iataCode}`
	}
	return result;
}

function parsePKPass(pass, node) {
	const res = node.result[0];

	// Vueling had an awesome idea to surface the boarding group in an image...
	// other SSRs, and priority boarding are shown in a image too...

	res.reservationFor.boardingTime = JsonLd.toDateTime(
		`${res.reservationFor.departureDay} ${pass.field["departureboarding"].value.replace("h", "")}`,
		"yyyy-MM-dd hh:mm",
		"en"
	);
	
	let [_d, departureTime, departureTerminal] = /· Departure (\d+:\d+)h(?: - Terminal (.*))?/gm.exec(pass.field["flightinfo"])
	res.reservationFor.departureTime = JsonLd.toDateTime(
		`${res.reservationFor.departureDay} ${departureTime}`,
		"yyyy-MM-dd h:mm",
		"en"
	);
	res.reservationFor.departureTerminal = departureTerminal || res.reservationFor.departureTerminal;

	let [_a, arrivalTime, arrivalTerminal] = /· Arrival (\d+:\d+)h(?: - Terminal (.*))?/gm.exec(pass.field["flightinfo"])
	res.reservationFor.arrivalTime = JsonLd.toDateTime(
		`${res.reservationFor.departureDay} ${arrivalTime}`,
		"yyyy-MM-dd h:mm",
		"en"
	);
	res.reservationFor.arrivalTerminal = arrivalTerminal || res.reservationFor.arrivalTerminal;

	let [_eb, endBoardingMinutes] = /· End of boarding: (\d+) min before departure./.exec(pass.field["flightinfo"])
	res.reservationFor.departureGateCloseTime = addMinutes(res.reservationFor.departureTime["@value"], -parseInt(endBoardingMinutes));
	
	return res
}

/**
 * @param {string} date 
 * @param {number} minutes 
 * @returns {Date}
 */
function addMinutes(date, minutes) {
    return new Date((new Date(date)).getTime() + minutes*60000);
}