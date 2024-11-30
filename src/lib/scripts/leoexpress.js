// SPDX-FileCopyrightText: 2024 Volker Krause <vkrause@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

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
    const text = pdf.pages[0].text;
    const dep = text.match(/(\d\d:\d\d \d{1,2}\.\d{1,2}\.\d{2}) \[(.*?)\] +/);
    const leg = text.match(/ +([^\s\d\[\]].*\S) -> (\S.*\S) \((\d{1,2}\.\d{1,2}\.\d{2} \d\d:\d\d)/);
    let res = JsonLd.newTrainReservation();
    res.reservationFor.departureTime = JsonLd.toDateTime(dep[1], "hh:mm d.M.yy", "sk");
    res.reservationFor.trainNumber = dep[2];
    res.reservationFor.departureStation.name = leg[1];
    res.reservationFor.arrivalStation.name = leg[2];
    res.reservationFor.arrivalTime = JsonLd.toDateTime(leg[3], "d.M.yy hh:mm", "sk");

    const hdr = text.match(/^(\S.*\S)  +(\d{5}-\d{5}-\d{5})/);
    res.underName.name = hdr[1];
    res.reservationNumber = hdr[2].replace(/-/g, '');
    res.reservedTicket.ticketToken = 'qrcode:' + res.reservationNumber;
    return res;
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