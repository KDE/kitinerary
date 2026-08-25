// SPDX-FileCopyrightText: 2026 David Pilarčík <meow@charliecat.space>
// SPDX-License-Identifier: LGPL-2.0-or-later

function parsePassCode(contents) {
	const res = JsonLd.newEventReservation()
	let ticket = createObjectFromCodeContents(contents)

	res.reservedTicket.ticketToken = "qrCode:" + contents
	res.reservationNumber = ticket["A"]
	res.reservationFor.startTime = JsonLd.toDateTime(ticket["F"] + ticket["H"], 'yyMMddhhmm', 'es');
	res.underName.name = ticket["K"].split(":")[1]
	res.reservationFor.name = "Ticket for the Alhambra"
	res.reservedTicket.ticketNumber = `${ticket["A"]}/${parseInt(ticket["N"])}`
	
	return res
}

/**
 * 
 * @param {string} contents 
 * @returns {{[key: string]: `${string}` | `${string}:${string}`}}
 */
function createObjectFromCodeContents(contents) {
	// /OI/P116/I0/AABCDEFG/E449/N001/B120/J4/FyyMMdd/Hhhmm/KXX0000000:First1 Last/G23/R1:Y13-FyyMMdd-Hhhmm
	// /OI/P116/I0/AABCDEFG/E449/N002/B120/J4/FyyMMdd/Hhhmm/KXX0000000:First2 Last/G24/R1:Y13-FyyMMdd-Hhhmm
	// /OI/P116/I0/AABCDEFG/E449/N001/B120/J4/FyyMMdd/Hhhmm/KXX0000000:First3 Last/G24/R1:Y13-FyyMMdd-Hhhmm
	// /OI/P116/I0/AABCDEFG/E431/N005/B001/J1/FyyMMdd/Hhhmm/KXX0000000:First4 Last/G15/R1:Y17-FyyMMdd-Hhhmm - https://www.scribd.com/document/533475021/Voucher-GYGZG3YQFAF3
	// /OI/P116/I0/AABCDEFG/E431/N006/B001/J1/FyyMMdd/Hhhmm/KXX0000000:First5 Last/G16/R1:Y17-FyyMMdd-Hhhmm - https://www.scribd.com/document/533475021/Voucher-GYGZG3YQFAF3
	// /OI/P116/I0/AABCDEFG/E432/N001/B010/J1/FyyMMdd/Hhhmm/KXX0000000:First6 Last/G22/R1:Y21-FyyMMdd-Hhhmm - https://www.scribd.com/document/1066715014/H0LVONH
	// O = unknown
	// P = unknown --- shows as first part of "groupingIdentifier" & "serialNumber" in pkpass
	// I = unknown
	// A = PNR eq. "Localisator"
	// E = type of ticket?
	// N = number of the ticket in the reservation
	// B = type of visit?
	// J = nunmber of allowed enters?
	// F = date of the ticket validity `yyMMdd`
	// H = access time to the nasarid palaces `hhmm`
	// K = `{Identity card SNR}:{Full Name}`
	// G = something like sequence number?
	// R = `1:Y{unknown}-F{date}-H{access time}`
	let object = {}
	let segments = contents.substr(1).split("/")
	for (let seg of segments) {
		let [_, key, value] = seg.match(/([A-Z])(.*)/)
		object[key] = value
	}

	return object
}

// TODO: full pdf tickets
