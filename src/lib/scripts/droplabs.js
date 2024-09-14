function parsePass(pass, node) {

	const res = Object.assign(JsonLd.newEventReservation(), node.result[0])

	res.reservationFor.startDate = JsonLd.toDateTime(pass.field['validity'].value, "dd.mm.yyyy", "sk") // 09.08.2024
	res.reservationFor.name = pass.field["ticket"].value ?? res.reservationFor.name

	//const loc = res.location = JsonLd.newObject("PostalAddress")
	//res.location.address = pass.field['address'].value

	res.underName = { '@type': 'Person', email: pass.field['email'].value }
	res.provider = { '@type': 'Organization', name: pass.field['seller'].value };
	res.reservationNumber = pass.barcodes[0].alternativeText


	return res
}

// function parsePDF(pdf, node, barcode) {

// 	console.log(pdf.text)
// 	console.log('===================')
	
// 	return node.result[0]
// }