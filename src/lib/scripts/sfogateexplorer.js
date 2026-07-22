/*
   SPDX-FileCopyrightText: ⓒ 2026 David Pilarčík <meow@charliecat.space>
   SPDX-License-Identifier: LGPL-2.0-or-later
*/

function parseBCBP(bcbp, node) {
	const res = JsonLd.newEventReservation()
	const parsedFlight = node.result[0]

	res.reservationNumber = parsedFlight.reservationNumber
	res.reservationFor.name = "SFO Gate Explorer"
	res.reservationFor.startDate = parsedFlight.reservationFor.departureDay
	res.underName = parsedFlight.underName
	res.reservationFor.geo = parsedFlight.reservationFor.geo
	res.reservedTicket = parsedFlight.reservedTicket

	return res
}