// SPDX-FileCopyrightText: 2026 David Pilarčík <meow@charliecat.space>
// SPDX-License-Identifier: LGPL-2.0-or-later

function parseEmailReservation(html) {
	// TaxiReservation isnt supported by kItinerary yet
	const res = {
		"@context": "https://schema.org",
		"@type": "TaxiReservation",
		"reservationStatus": "https://schema.org/ReservationConfirmed",
		"reservationFor": {
			"@type": "TaxiService",
			"provider": {
				"@type": "Organization",
				"name": "Uber"
			}
		},
	}

	ExtractorEngine.extractPrice(html.eval('//html/body')[0].recursiveContent, res);
	res.underName = JsonLd.newObject("Person");
	res.underName.givenName = html.eval('//html/body/table/tbody/tr/td/table/tbody/tr/td/table[2]/tbody/tr/td/table/tbody/tr/td/table/tbody/tr/td/table/tbody/tr/td/h2')[0].content.match(/(.*), .*/)[1]
	res.reservationFor.name = html.eval('//html/body/table/tbody/tr/td/table/tbody/tr/td/table[3]/tbody/tr/td/table/tbody/tr/td/table/tbody/tr/td/table/tbody/tr/td[2]/table/tbody/tr[1]/td/table/tbody/tr/td/table[1]/tbody/tr/td/table/tbody/tr/td[1]')[0].content
	res.partySize = html.eval('/html/body/table/tbody/tr/td/table/tbody/tr/td/table[3]/tbody/tr/td/table/tbody/tr/td/table/tbody/tr/td/table/tbody/tr/td[2]/table/tbody/tr[1]/td/table/tbody/tr/td/table[1]/tbody/tr/td/table/tbody/tr/td[4]')[0].content
	res.pickupLocation = JsonLd.newObject("Place");
	res.pickupLocation.name = html.eval('/html/body/table/tbody/tr/td/table/tbody/tr/td/table[5]/tbody/tr/td/table/tbody/tr/td/table/tbody/tr/td/table/tbody/tr/td/table/tbody/tr/td/table/tbody/tr/td/table[1]/tbody/tr/td/table/tbody/tr/td/table/tbody/tr/td/table/tbody/tr/td/table[1]/tbody/tr/td[6]/table/tr[2]/td/div')[0].content
	res.pickupLocation.address = JsonLd.newObject("PostalAddress");
	res.pickupLocation.address.streetAddress = html.eval('/html/body/table/tbody/tr/td/table/tbody/tr/td/table[5]/tbody/tr/td/table/tbody/tr/td/table/tbody/tr/td/table/tbody/tr/td/table/tbody/tr/td/table/tbody/tr/td/table[1]/tbody/tr/td/table/tbody/tr/td/table/tbody/tr/td/table/tbody/tr/td/table[1]/tbody/tr/td[6]/table/tr[3]/td')[0].content
	res.dropoffLocation = JsonLd.newObject("Place");
	res.dropoffLocation.name = html.eval('/html/body/table/tbody/tr/td/table/tbody/tr/td/table[5]/tbody/tr/td/table/tbody/tr/td/table/tbody/tr/td/table/tbody/tr/td/table/tbody/tr/td/table/tbody/tr/td/table[1]/tbody/tr/td/table/tbody/tr/td/table/tbody/tr/td/table/tbody/tr/td/table[2]/tbody/tr/td[6]/table/tr[2]/td/div')[0].content
	res.dropoffLocation.address = JsonLd.newObject("PostalAddress");
	res.dropoffLocation.address.streetAddress = html.eval('/html/body/table/tbody/tr/td/table/tbody/tr/td/table[5]/tbody/tr/td/table/tbody/tr/td/table/tbody/tr/td/table/tbody/tr/td/table/tbody/tr/td/table/tbody/tr/td/table[1]/tbody/tr/td/table/tbody/tr/td/table/tbody/tr/td/table/tbody/tr/td/table[2]/tbody/tr/td[6]/table/tr[3]/td')[0].content
	let editReservationUrl = new URL(html.eval('//a[contains(text(), "Edit reservation")]')[0].attribute('href'))
	res.modifyReservationUrl = decodeURIComponent(editReservationUrl.searchParams.get("_fallback_redirect") ?? "")
	res.reservationNumber = res.modifyReservationUrl.match(/[a-fA-F\d]{8}(?:\-[a-fA-F\d]{4}){3}\-[a-fA-F\d]{12}/)[0]
	return res;
}
