// SPDX-FileCopyrightText: 2026 David Pilarčík <meow@charliecat.space>
// SPDX-License-Identifier: LGPL-2.0-or-later

function parseEmail(html) {
	const baseRes = JsonLd.newEventReservation();
	baseRes.reservationFor.name = (html.eval("//div[contains(@style,'font-size:28px')]")[0]).content.replace("You have signed up for", "")
	baseRes.reservationFor.url = (html.eval("//a[normalize-space(.)='View Event']/@href")[0]).content
	let eventTimezone = html.eval("//div[starts-with(normalize-space(.), 'All times are for')]")[0].content.replace("All times are for", "")

	const results = []
	const reservedSessions = html.eval("//td[contains(@class,'session')]")
	for (let session of reservedSessions) {
		let res = JsonLd.clone(baseRes)

		let sessionDate = session.eval("//span[contains(@class,'date')]")[0].content // "Friday, 12 June 2026"
		let sessionTime = session.eval("//span[contains(@class,'time')]")[0].content.split(" • ") // "14:00 • 1 h" => ["14:00", "1 h"]
		let sessionLocation = session.eval("//span[contains(@class,'location')]")[0].content // "location name"

		res.reservationFor.location.name = sessionLocation
		res.reservationFor.startDate = JsonLd.toDateTime(
			`${sessionDate} ${sessionTime[0]} ${eventTimezone}`,
			"dddd, d MMMM yyyy h:mm  tttt",
			"en"
		)
		res.reservationFor.endDate = getSessionEndTime(res.reservationFor.startDate, sessionTime[1])
		
		
		results.push(res)
	}
	
	return results
}

function getSessionEndTime(startTime, duration) {
	const [_, days, hours, minutes] = /(?:(\d+) d)?(?:(\d+) h)?(?:(\d+) m)?/g.exec(duration) // 1 d 1 h 30 m
	return new Date(
		startTime.getTime()
		+ ((days ? days : 0) * 24 * 60 * 60 * 1000)
		+ ((hours ? hours : 0) * 60 * 60 * 1000)
		+ ((minutes ? minutes : 0) * 60 * 1000)
		
	)
}