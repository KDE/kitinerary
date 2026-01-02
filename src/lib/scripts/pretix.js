/*
   SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
   SPDX-License-Identifier: LGPL-2.0-or-later
*/

function readDateTime(pass, fieldName) {
    const f = pass.field[fieldName];
    if (!f)
        return undefined;
    const v = f.value;
	let valueAsDateObject = new Date(v)
	if (!isNaN(valueAsDateObject.getTime())) 
		return f.value;
    if (typeof v === "string")
        return JsonLd.toDateTime(v, ['dd.MM.yyyy hh:mm', 'dd.MM.yyyy' ], 'de');
    return v;
}

function parsePass(content, node) {
    let res = node.result[0];

    res.reservationFor.name = content.field['eventName'].value;

    res.reservationFor.startDate = readDateTime(content, 'doorsOpen');
    res.reservationFor.endDate = readDateTime(content, 'doorsClose');
    res.reservationFor.doorTime = readDateTime(content, 'doorsAdmission');
	res.bookingTime = readDateTime(content, 'purchaseDate');
    
	res.reservationFor.url = content.field['website'].value;
    
	if (content.field['name']) {
        res.underName = JsonLd.newObject('Person');
        res.underName.name = content.field['name'].value;
		res.underName.email = content.field['email']?.value;
    
	}
    res.reservationNumber = content.field['orderCode'].value;
    res.reservedTicket.name = content.field['ticket'].value;
	
	res.provider = {
		"@type": "Organization",
		name: content.field['organizer'].value,
		email: content.field['organizerContact']?.value,
	}
    
	return res;
}

// only works for unstyled PDFs common for smaller events
function parsePdfPage(pdf, page, barcode) {
    let res = JsonLd.newEventReservation();
    const text = pdf.pages[page].textInRect(0.0, 0.0, 1.0, 0.4);
    const dt = text.match(/(\d{4}.\d\d.\d\d \d\d:\d\d|\d\d.\d\d.\d{4} \d\d:\d\d|\d{1,2}\. \S+ \d{4})/);
    res.reservationFor.startDate = JsonLd.toDateTime(dt[1], ['dd.MM.yyyy hh:mm', 'yyyy-MM-dd hh:mm', 'dd. MMMM yyyy'], ['en', 'de']);

    const data1 = text.substr(0, dt.index).trim().split(/\n+/);
    res.reservationFor.name = data1.slice(0, Math.max(data1.length - 2, 1)).join(' ');

    const data2 = text.substr(dt.index + dt[0].length).trim().split(/\n+/);
    if (data2 && data2.length > 0 && data2[0]) {
        res.reservationFor.location.name = data2.slice(0, data2.length - 1).join(' ');
        res.reservationNumber = data2[data2.length - 1].match(/(\S+) /)[1];
    }

    res.reservedTicket.ticketToken = 'qrcode:' + barcode;
    return res;
}

function parsePdf(pdf, node) {
    let reservations = [];
    for (const ticket of node.findChildNodes({ mimeType: "application/octet-stream", scope: "Descendants"})) {
        reservations.push(parsePdfPage(pdf, ticket.location, ticket.content));
    }
    return reservations;
}

function parsePdfWithBarcode(pdf, node, barcode) {
    return parsePdfPage(pdf, barcode.location, barcode.content);
}
