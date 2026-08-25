// SPDX-FileCopyrightText: 2026 David Pilarčík <meow@charliecat.space>
// SPDX-License-Identifier: LGPL-2.0-or-later

function parseEmailTicket(html) {
	const res = JsonLd.newEventReservation()

	ExtractorEngine.extractPrice(html.eval('//html/body')[0].recursiveContent, res);
	res.reservedTicket.ticketNumber = html.eval('//h3[contains(text(), "Ticket número")]')[0].content.match(/\d+/)[0]
	res.reservedTicket.ticketToken = "code128:" + res.reservedTicket.ticketNumber
	res.reservationNumber = parseInt(res.reservedTicket.ticketNumber)
	res.underName.name = html.eval('//p[starts-with(normalize-space(.), "Cliente:")]')[0].content.split(":")[1].trim()

	let venueName = html.eval('//table[not(.//table)][.//td[@class="aligncenter"]]/tr/td[1]')[0].content
	// example: "10/08/2026"
	let visitDate = html.eval('//table[not(.//table)][.//td[@class="aligncenter"]]/tr/td[@class="aligncenter"]')[0].content
	// example: "Mañana 10:00 - 14:00"
	// ["Mañana 10:00 - 14:00", "10:00", "14:00"]
	let [visitTimeSlotName, visitStartTime, visitDoorsClose] =
		/.* (\d+:\d+) - (\d+:\d+)/.exec(html.eval('//table[not(.//table)][.//td[@class="aligncenter"]]/tr/td[@class="alignright"]')[0].content)
	res.reservationFor.name = venueName
	res.reservationFor.location.name = venueName
	res.reservedTicket.name = `${venueName} - ${visitTimeSlotName}`
	res.reservationFor.startDate = JsonLd.toDateTime(`${visitDate} ${visitStartTime}`, ["dd/MM/yyyy hh:mm"], ["es"])
	res.reservationFor.doorsClose = JsonLd.toDateTime(`${visitDate} ${visitDoorsClose}`, ["dd/MM/yyyy hh:mm"], ["es"])
	
	return res
}