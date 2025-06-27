// SPDX-FileCopyrightText: 2024 Volker Krause <vkrause@kde.org>
// SPDX-FileCopyrightText: 2024-2025 David Pilarcik <meow@charliecat.space>
// SPDX-License-Identifier: LGPL-2.0-or-later

const i18n = {
	"en": {
		__lookingFor: "TICKET/TAX RECEIPT",
		"extracting name - TICKET/TAX RECEIPT": /(.*) +TICKET\/TAX RECEIPT/,
		"(train connection) string": "(train connection)",
		"Class": "CLASS",
	},
	"sk": {
		__lookingFor: "LÍSTOK/DAŇOVÝ DOKLAD",
		"extracting name - TICKET/TAX RECEIPT": /(.*) +LÍSTOK\/DAŇOVÝ DOKLAD/,
		"(train connection) string": "(vlakové spojenie)",
		"Class": "TRIEDA",
	},
	"cs": {
		__lookingFor: "JÍZDENKA/DAŇOVÝ DOKLAD",
		"extracting name - TICKET/TAX RECEIPT": /(.*) +JÍZDENKA\/DAŇOVÝ DOKLAD/,
		"(train connection) string": "(vlakové spojení)",
		"Class": "TŘÍDA",
	}
}

function extractEvent(ev, node) {
    let res = JsonLd.newTrainReservation();
    res.reservationFor.departureStation.name = ev.location;
    res.reservationFor.arrivalStation.name = ev.summary.match(/→ (.*)/)[1];
    res.reservationFor.departureTime = node.result[0].startDate;
    res.reservationFor.arrivalTime = node.result[0].endDate;
    res.reservationFor.provider.name = ev.organizer.name;
    res.reservationFor.provider.url = ev.url;
    res.reservationFor.trainNumber = ev.description.match(/^(\d+) /)[1];
    res.reservationNumber = ev.description.match(/\n(\d{15})\n/)[1];
    res.reservedTicket.ticketToken = 'qrcode:' + res.reservationNumber;
    return res;
}

function extractPdf(pdf) {
    const text = pdf.text;
	const parts = text.split(/(?=\n([A-Ž]*\s)+ +\d{5}-\d{5}-\d{5})/g)
	const result = [];

	let lang = i18n["en"];
	for (let l in i18n) {
		if (text.includes(i18n[l].__lookingFor)) {
			lang = i18n[l];
			break;
		}
	}

	for (let part of parts) {
		if (!part.includes("->")) continue;

		const dep = part.match(/(\d\d:\d\d \d{1,2}\.\d{1,2}\.\d{2}) \[(.*?)\] +/);
		const leg = part.match(/ +([^\s\d\[\]].*\S) -> (\S.*\S) \((\d{1,2}\.\d{1,2}\.\d{2} \d\d:\d\d)/);

		let res = (part.includes(lang["(train connection) string"])) ? JsonLd.newTrainReservation() : JsonLd.newBusReservation();

		if (part.includes(lang["(train connection) string"])) {
			res.reservationFor.trainNumber = dep[2];
			res.reservationFor.departureStation.name = leg[1];
			res.reservationFor.arrivalStation.name = leg[2];
		} else {
			res.reservationFor.departureBusStop.name = leg[1];
			res.reservationFor.arrivalBusStop.name = leg[2];
			res.reservationFor.busNumber = dep[2];
		}

		res.reservationFor.departureTime = JsonLd.toDateTime(dep[1], "hh:mm d.M.yy", "sk");
		res.reservationFor.arrivalTime = JsonLd.toDateTime(leg[3], "d.M.yy hh:mm", "sk");
		const hdr = part.match(/((?:[A-Ž]*\s){2,}) +(\d{5}-\d{5}-\d{5})/);
		res.underName.name = hdr[1];
		res.underName.email = /^(.*@.* ) +/gm.exec(part)?.[1].trim() || undefined;
		res.reservationNumber = hdr[2].replace(/-/g, '');
		res.reservedTicket.ticketToken = 'qrcode:' + res.reservationNumber;
		res.reservedTicket.name = lang["extracting name - TICKET/TAX RECEIPT"].exec(part)[1];
		res.reservedTicket.ticketedSeat.seatNumber = part.match(/\s+([A-Z]\d{2,3})/)?.[1] || undefined;

		// This thing is done due to the length of the Class Cell being short for some Class Names
		let partLines = part.split("\n");
		let classHeaderLine = partLines.findIndex(line => line.includes(lang["Class"]));
		let classHeaderIndex = partLines[classHeaderLine].indexOf(lang["Class"]);
		res.reservedTicket.ticketedSeat.seatingType = ""
		for (let i = classHeaderLine + 1; i < (classHeaderLine + 4); i++) {
			let className = partLines[i].substring(classHeaderIndex, classHeaderIndex + 15).trim() || " ";
			console.log(partLines[i].substring(classHeaderIndex, classHeaderIndex + 15))
			if (className) res.reservedTicket.ticketedSeat.seatingType += className;
		}

		ExtractorEngine.extractPrice(part, res);
		result.push(res);
	}
    return result;
}

function extractPkpass(pass, node) {
	const res = Object.assign(JsonLd.newTrainReservation(), node.result[0]);

	res.reservationFor.departureStation.name = pass.field["departure"].label;
    res.reservationFor.departureTime = JsonLd.toDateTime(pass.field["departure"].value + " " + pass.field["departureDate"].value, "hh:mm dd.MM.yy", "cz");

    res.reservationFor.arrivalStation.name = pass.field["arrival"].label;
    res.reservationFor.departureTime = JsonLd.toDateTime(pass.field["arrival"].value + " " + (pass.field["arrivalDate"] ? pass.field["arrivalDate"].value : pass.field["departureDate"].value), "hh:mm dd.MM.yy", "cz");

    res.reservationFor.trainNumber = pass.field["line"].value
    res.reservationNumber = pass.field["code"].value

	res.reservedTicket.name = pass.field["rate"].value
	res.reservedTicket.ticketedSeat = {
        "@type": "Seat",
        seatNumber: pass.field["seat"].value,
		seatingType: pass.field["className"].value,
    }

	return res
}