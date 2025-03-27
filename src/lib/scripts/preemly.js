// SPDX-FileCopyrightText: 2025 David Pilarčík <meow@charliecat.space>
// SPDX-License-Identifier: LGPL-2.0-or-later

function extractMail(content, node) {
	const res = JsonLd.newEventReservation();

	res.reservationFor.name = /(.*) - Preemly\s/gm.exec(content)[1]
	res.underName.name = /Here's your ticket, (.*)\s/gm.exec(content)[1]

	// March 26, 2025 17:00
	let [_eventStartTime, startDate] = /Date & Time\s(.* \d+, \d+ \d+:\d+)/gm.exec(content)
	res.reservationFor.startDate = JsonLd.toDateTime(startDate, "MMM dd, yyyy HH:mm", "en")

	// Preemly allows hosts to set custom location name
	// in sample "FEI STU, Bratislava"
	res.reservationFor.location.name = /Location\s(.*)\s/gm.exec(content)[1]

	// QR Code is fetched from a 3rd-party server as an on-demand image
	// sample: https://api.qrserver.com/v1/create-qr-code/?data=67e2e31e794f6aba64a69521&size=320x320&color=000000&bgcolor=FFFFFF
	let _html = node.parent.content.rawData
	let qrCodeURL = new URL(/https:\/\/api.qrserver.com\/[a-zA-z0-9&=?\-\/]+/gm.exec(_html)[0])
	res.reservedTicket.ticketToken = 'qrCode:' + qrCodeURL.searchParams.get("data")

	return res
}