/*
    SPDX-FileCopyrightText: 2024 David Pilarćík <meow@charliecat.space>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

function parsePkPass(pass, node) {
    const res = node.result[0];

    let _price = (pass.field["price"]?.value ?? "0 ").split(" ")
    res.totalPrice = Number(_price[0])
    res.priceCurrency = _price[1]

	res.reservationFor.location = Object.assign(
		JsonLd.newObject("Place"), 
		{ name: pass.field["hall"]?.value ?? pass.field["location"]?.value ?? pass.organizationName }
	);

    res.reservationFor.name = pass.field["eventname"].value
    res.reservedTicket.name = pass.field["section"].value
    if (pass.field["seat"]) {
        res.reservedTicket.ticketedSeat = {
            '@type': 'Seat',
            seatNumber: pass.field["seat"].value,
            seatRow: pass.field["row"].value,
            seatSection: pass.field["section"].value
        }
    }

    res.reservationNumber = pass.barcodes[0].alternativeText;

    return res
}

function parsePdf(pdf, node, triggerNode) {
    const results = []

    for (let i = 0; i < pdf.pages.length; i++) {
        const page = pdf.pages[i]
        let res = JsonLd.newEventReservation();

        res.reservationFor.name = page.textInRect(0.0, 0, 0.5, 0.15);
        res.reservationFor.startDate = JsonLd.toDateTime(page.textInRect(0.0, 0.15, 0.5, 0.18), "dd.MM.yyyy", "sk") // 29.11.2024
        res.reservationNumber = page.textInRect(0.5, 0, 1, 0.3).replace("Tisk Colosseum", "").trim()
        res.reservedTicket.name = page.textInRect(0, 0.16, 0.6, 0.2)

        let _price = page.textInRect(0.5, 0.3, 1, 0.5).replace("Cena: ").split(" ") // [ "5", "EUR" ]
        res.totalPrice = Number(_price[0])
        res.priceCurrency = _price[1]

        let barcode = node.findChildNodes({ mimeType: "text/plain", match: ".*", scope: "Descendants" }).filter(n => n.parent.mimeType == "internal/qimage")[i]
        res.reservedTicket.ticketToken = 'qrCode:' + barcode.content

        results.push(res)
    }

    return results
}

function parseSNDQRCode(qrCode) {
	// found example: "160c2706f65982ce#00#892149743623#LA FILLE MAL GARDÉE/ ZLE STRÁŽENÉ DIEVČA#202506081800#7#22##1#BALKÓN\u0000"
	// Probably the meaning of the fields:
	// 0: checksum probably
	// 1: unknown
	// 2: reservation number
	// 3: event name
	// 4: date and time
	// 5: seat row
	// 6: seat number
	// 7: unknown
	// 8: unknown
	// 9: seat section
	const parts = qrCode.replace(/\\u0000/g, '').split("#")
	let res = JsonLd.newEventReservation()

	res.reservationNumber = parts[2]
	res.reservationFor.name = parts[3]
	res.reservationFor.startDate = JsonLd.toDateTime(parts[4], "yyyyMMddHHmm", "sk")
	res.reservedTicket.seatRow = parts[5]
	res.reservedTicket.seatNumber = parts[6]
	res.reservedTicket.ticketedSeat = {
		'@type': 'Seat',
		seatNumber: parts[6],
		seatRow: parts[5],
		seatSection: parts[9]
	}
	res.reservedTicket.ticketToken = 'qrCode:' + qrCode

	return res
}
