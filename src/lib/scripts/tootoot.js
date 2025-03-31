function extractPdf(pdf, node, barcode) {
	const res = JsonLd.newEventReservation();
	const page = pdf.pages[barcode.location];

	res.reservationFor.name = page.textInRect(0.0, 0.0, 1, 0.05)
	res.reservedTicket.ticketToken = 'qrCode:' + barcode.content;

	let timeSeatType = page.textInRect(0, 0.05, 0.25, 0.15).split('\n'); // [ Time, Sector, Seat ]
	let timeExtracted = pdfTimeHandle(timeSeatType[0]);
	res.reservedTicket.ticketedSeat.seatNumber = timeSeatType[2] ? timeSeatType[2].replace('Seat:', '').trim() : null;
	res.reservedTicket.ticketedSeat.seatSection = timeSeatType[1] ? timeSeatType[1].replace('Sector:', '').trim() : null;
	if (timeExtracted[1]) {
		res.reservationFor.startDate = JsonLd.toDateTime(timeExtracted[0], timeExtracted[0].includes(':') ? 'dd/MM/yyyy HH:mm' : 'dd/MM/yyyy', 'en');
		res.reservationFor.endDate = JsonLd.toDateTime(timeExtracted[1], timeExtracted[1].includes(':') ? 'dd/MM/yyyy HH:mm' : 'dd/MM/yyyy', 'en');
	} else {
		res.reservationFor.startDate = JsonLd.toDateTime(timeExtracted[0], timeExtracted[0].includes(':') ? 'dd/MM/yyyy HH:mm' : 'dd/MM/yyyy', 'en');
	}

	let locationInfo = page.textInRect(0.25, 0.05, 1, 0.15).split('\n'); // [ Optional(VenueName), VenueStreet, VenueCity ]
	if (locationInfo.length >= 3) {
		res.reservationFor.location.name = locationInfo[0].trim();
		res.reservationFor.location.address.streetAddress = locationInfo[1].trim();
		res.reservationFor.location.address.addressLocality = locationInfo[2].trim();
	} else if (locationInfo.length == 2) {
		res.reservationFor.location.address.streetAddress = locationInfo[0].trim();
		res.reservationFor.location.address.addressLocality = locationInfo[1].trim();
	}

	let organizerInfo = page.textInRect(0.0, 0.15, 0.25, 0.25).split('\n'); // [ "Organizer:", OrganizerName, ... ]
	res.provider = { '@type': 'Organization', name: organizerInfo[1] };


	ExtractorEngine.extractPrice(page.text, res);

	return res
}

function pdfTimeHandle(timeString) {
	let isMultiday = timeString.includes(' - ');

	const resulting = []

	if (isMultiday) {
		let time = timeString.split(' - ');
		let startDate = /\d+\/\d+\/\d+/gm.exec(time[0])[0];
		let endDate = /\d+\/\d+\/\d+/gm.exec(time[1])?.[0];
		let startTime = /\d+:\d+/gm.exec(time[0])[0];
		let endTime = /\d+:\d+/gm.exec(time[1])?.[0];

		resulting.push((startDate + ' ' + startTime).replace('undefined', '').trim());
		resulting.push((endDate + ' ' + endTime).replace('undefined', '').trim());
	} else {
		let time = timeString.split(' - ');
		let startDate = /\d+\/\d+\/\d+/gm.exec(time[0])[0];
		let startTime = /\d+:\d+/gm.exec(time[0])[0];

		resulting.push((startDate + ' ' + startTime).replace('undefined', '').trim());
		resulting.push(null);
	}

	return resulting
}