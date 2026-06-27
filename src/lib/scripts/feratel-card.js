/*
   SPDX-FileCopyrightText: 2021 Kai Uwe Broulik <kde@broulik.de>
   SPDX-FileCopyrightText: 2026 David Pilarčík <meow@charliecat.space>
   SPDX-License-Identifier: LGPL-2.0-or-later
*/

function parseReservation(pass, node) {
	let res = node.result[0];

	res.reservationFor.name = pass.description
	res.reservationNumber = pass.serialNumber

	const cardName = pass.field["cardName"] ?? pass.field['customerName']
    const ausgabestelleName = pass.field["ausgabestelleName"];

    if (cardName && cardName.value !== "- -") {
        res.underName = JsonLd.newObject("Person");
        res.underName.name = cardName.value;
    }
    if (ausgabestelleName) {
        res.reservationFor.name = ausgabestelleName.value;
	}
    
    const cardValidFrom = pass.field["cardValidFrom"] ?? pass.field["validFrom"]
    const cardValidTo = pass.field["cardValidTo"] ?? pass.field["validTo"]

    if (cardValidFrom) {
		res.reservationFor.startDate = cardValidFrom.value;
		res.reservedTicket.validFrom = cardValidFrom.value;
    }
    if (cardValidTo) {
		res.reservationFor.endDate = cardValidTo.value;
        res.reservedTicket.validUntil = cardValidTo.value;
    }

    if (pass.field['web']) {
        res.reservationFor.url = pass.field['web'].value;
	}

	const ticketAgeCategory = pass.field['cardPriceCategory']
    res.reservedTicket.name = !!ticketAgeCategory ? ticketAgeCategory.value : ""

    return res;
}
