const i18n = {
	"sk-SK": {
		__lookingFor: "Kód slúži na jednoznačnú identifikáciu",
		"New reservation created for": /Vytvorenie novej rezervácie\s(.*)/gm,
		"which is under name": /ktorá je na meno (.*)./gm,
		"date and time": /Termín\s.* (\d+.\d+.\d+) (\d+:\d+) – (\d+:\d+)/gm,
		"where?": /KDE\?\s(.*)\s(.*)/gm
	}
}

function parseMail(html) {
	const res = JsonLd.newEventReservation();
	const text = html.root.recursiveContent

	let lang = i18n["sk-SK"]

	for (let langMutation in i18n) {
		if (text.includes(i18n[langMutation].__lookingFor)) {
			lang = i18n[langMutation]
			break
		}
	}

	res.reservationFor.name = lang["New reservation created for"].exec(text)[1];

	let reservationCode = /([A-Z0-9]{3}-[A-Z0-9]{3}-[A-Z0-9]{3})/gm.exec(text)[0];
	res.reservationNumber = reservationCode;
	res.reservedTicket.ticketToken = "qrCode:" + reservationCode

	let userName = lang["which is under name"].exec(text)[1];
	res.underName = {
		'@type': 'Person',
		name: String(userName ?? "").trim()
	};

	// example "Termín pondelok 19.5.2025 18:00 – 20:00"
	// d.m.yyyy HH:mm – HH:mm
	let [_d, date, startTime, endTime] = lang["date and time"].exec(text);
	res.reservationFor.startDate = JsonLd.toDateTime(date + ' ' + startTime, "dd.M.yyyy HH:mm", "sk");
	res.reservationFor.endDate = JsonLd.toDateTime(date + ' ' + endTime, "dd.M.yyyy HH:mm", "sk");

	// example "KDE?  Stará tržnica, Bratislava  Námestie SNP 25, Bratislava 811 01, Slovensko"
	// [ _a, Venue Name, Venue Address ] // Address can be asumed that is in Slovak or Czech
	let [_a, venueName, venueAddress] = lang["where?"].exec(text)
	res.reservationFor.location.name = venueName
	let [ streetAddress, municipalityAndPostal, Country ] = venueAddress.split(", ")
	res.reservationFor.location.address.streetAddress = streetAddress
	res.reservationFor.location.address.postalCode = municipalityAndPostal.match(/\d+ \d+/)[0]
	res.reservationFor.location.address.addressLocality = municipalityAndPostal.replace(/\d+ \d+/, "").trim()
	res.reservationFor.location.geo = JsonLd.toGeoCoordinates("https://maps.google.com?q=48.14484247668123,17.111600664436402")

	res.provider = { '@type': 'Organization', name: text.split("\n").slice(-3, -2)[0] };

	let webAccessToken = /\/.{13}\/\?accessToken=.{40}/gm.exec(text)[0]
	let reenioDomain = /\/\/reenio..*\/s.*\/r/gm.exec(text)[0]
	res.potentialAction = [
		{ "@type": "UpdateAction", "target": `https:${reenioDomain}/ReservationDetail${webAccessToken}` },
		{ "@type": "CancelAction", "target": `https:${reenioDomain}/CancelReservation${webAccessToken}` },
		{ "@type": "DownloadAction", "target": `https:${reenioDomain}/DownloadReservationCodePdf${webAccessToken}` },
	]
	
	return res
}

function extractPass(pass, node) {
	let res = node.result[0];

	res.reservationNumber = res.reservedTicket.ticketToken.split(":")[1]
	res.reservationFor.startDate = pass.field["startDate"].value

	// in first sample the name is "Stará tržnica, Bratislava"
	// in the second is "Uvedenie knihy Rozumne rastúce mesto, Stará tržnica, Bratislava"
	// this tries to split the name and the location
	let splitName = pass.field["TermName"].value.split(", ")
	res.reservationFor.location = JsonLd.newObject("Place")
	res.reservationFor.location.address = JsonLd.newObject("PostalAddress")
	if (splitName.length == 2) {
		res.reservationFor.location.name = splitName[0]
		res.reservationFor.location.address.addressLocality = splitName[1]
	} else if (splitName.length >= 3) {
		res.reservationFor.name = splitName.slice(0, -2).join(", ")
		res.reservationFor.location.name = splitName.slice(-2)[0]
		res.reservationFor.location.address.addressLocality = splitName.slice(-2)[1]
	}

	res.potentialAction = [
		{ "@type": "UpdateAction", "target": pass.field["detailUrl"].value },
		{ "@type": "CancelAction", "target": pass.field["detailUrl"].value.replace("ReservationDetail", "CancelReservation") },
		{ "@type": "DownloadAction", "target": pass.field["detailUrl"].value.replace("ReservationDetail", "DownloadReservationCodePdf") },
	]

	return res
}