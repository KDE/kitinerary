// SPDX-FileCopyrightText: 2025 David Pilarčík <meow@charliecat.space>
// SPDX-License-Identifier: LGPL-2.0-or-later

function parseMail(content, node, triggerNode) {
	const res =  JsonLd.newEventReservation();
	const text = content.root.recursiveContent

	// found in sample: `https://app.sportio.sk/reservation-detail/4`
	let [reservationDetailLink, reservationNumber] = content.rawData.match(/https:\/\/app.sportio.sk\/reservation-detail\/(\d+)/) ?? []
	res.reservationNumber = reservationNumber;
	res.modifyReservationUrl = reservationDetailLink;

	if (!reservationDetailLink) return [] // Ignores HTML and PDF Invoice
	if (text.startsWith("Rezervácia potvrdená")) { // Reservation has been confirmed
		let [_, venue, date, time] = /vaša rezervácia na športovisku (.*), dňa (\d{2}.\d{2}.\d{4}) o (\d{2}:\d{2})/.exec(text)
		res.reservationFor.startTime = JsonLd.toDateTime(date + " " + time, "dd.MM.yyyy hh:mm", "sk")
		res.reservationFor.location.name = venue
		res.reservationFor.name = venue;
	} else if (text.startsWith("Pridanie do rezervácie športoviska")) { // You have been invited into a reservation
		console.log(text)
		let [_o, organizator, venue] = /Sportio používateľ (.*) vás pridal do nadchádzajúcej športovej rezervácie na športovisku (.*)./.exec(text)
		let [_t, time, date] = /Rezervácia sa bude konať dňa (\d{2}.\d{2}.\d{4}) o (\d{2}:\d{2})/.exec(text)
		res.reservationFor.startTime = JsonLd.toDateTime(date + " " + time, "dd.MM.yyyy hh:mm", "sk")
		res.reservationFor.location.name = venue
		res.reservationFor.name = venue;
		res.provider = { "@type": "Person", name: organizator }
	} else return []

	return res
}