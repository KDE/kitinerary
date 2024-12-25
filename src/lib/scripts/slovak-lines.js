/*
   SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
   SPDX-FileCopyrightText: 2023 David Pilarčík <meow@charliecat.space>
   SPDX-License-Identifier: LGPL-2.0-or-later
*/

const regExMap = {};
regExMap['en_US'] = [];
regExMap['en_US']['departure'] = /Departure: +(.*) \S+, (\d\d\.\d\d\.\d{4} \d\d:\d\d)/;
regExMap['en_US']['arrival'] = /Arrival: +(.*) \S+, (\d\d\.\d\d\.\d{4} \d\d:\d\d)/;
regExMap['en_US']['platform'] = /Platform: +(\S.*)/;
regExMap['en_US']['seat'] = /Seat: +(\S.*)/
regExMap['en_US']['busLine'] = /Bus line no: +(.*) /
regExMap['en_US']['category'] = /Category: +(.*)/
regExMap['en_US']['brand'] = /Brand: \s+ +(.*)/

regExMap['sk_SK'] = [];
regExMap['sk_SK']['departure'] = /Odchod: +(.*) \S+, (\d\d\.\d\d\.\d{4} \d\d:\d\d)/;
regExMap['sk_SK']['arrival'] = /Príchod: +(.*) \S+, (\d\d\.\d\d\.\d{4} \d\d:\d\d)/;
regExMap['sk_SK']['platform'] = /Nástupište: +(\S.*)/;
regExMap['sk_SK']['seat'] = /Sedadlo: +(\S.*)/
regExMap['sk_SK']['busLine'] = /Autobusová linka č.: +(.*) /
regExMap['sk_SK']['category'] = /kategória: +(.*)/
regExMap['sk_SK']['brand'] = /Značka: \s+ +(.*)/

function extractPass(pass, node) {
    let res = node.result[0];
    res.reservationFor = JsonLd.newObject("BusTrip");
    res.reservationFor.departureTime =  JsonLd.toDateTime(pass.field["date"].value + ' ' + pass.field["origin"].value, "dd.MMM hh:mm", "en");
    res.reservationFor.departureBusStop = {
        '@type': 'BusStation',
        name: pass.field["origin_long"].value,
        address: {
            '@type': 'PostalAddress',
            addressLocality: pass.field["origin"].label
        },
        geo: {
            '@type': 'GeoCoordinates',
            latitude: pass.locations[0].latitude,
            longitude: pass.locations[0].longitude
        }
    }
    res.reservationFor.departurePlatform = pass.field["platform"].value;
    res.reservationFor.arrivalTime =  JsonLd.toDateTime(pass.field["date"].value + ' ' + pass.field["destination"].value, "dd.MMM hh:mm", "en");
    res.reservationFor.arrivalBusStop = {
        '@type': 'BusStation',
        name: pass.field["destination_long"].value,
        address: {
            '@type': 'PostalAddress',
            addressLocality: pass.field["destination"].label
        }
    };
    res.reservedTicket.ticketedSeat = {
        '@type': 'Seat',
        seatNumber: pass.field["tariff"].value.match(/\/(.*)/)[1]
    };
    return res;
}

function extractTicket(pdf, node, barcode) {
    const text = pdf.pages[barcode.location].text;
    let res = JsonLd.newBusReservation();
    res.underName.name = text.match(/^.*?: +(.*?)  /)[1];
    res.reservationNumber = barcode.content;
    
	for (let locale in regExMap) {
		if (!text.match(regExMap[locale]['departure']))
			continue

		const dep = text.match(regExMap[locale]['departure']);
    	res.reservationFor.departureBusStop.name = dep[1];
    	res.reservationFor.departureTime = JsonLd.toDateTime(dep[2], 'dd.MM.yyyy hh:mm', 'en');

    	const arr = text.match(regExMap[locale]['arrival']);
    	res.reservationFor.arrivalBusStop.name = arr[1];
    	res.reservationFor.arrivalTime = JsonLd.toDateTime(arr[2], 'dd.MM.yyyy hh:mm', 'en');

    	res.reservationFor.departurePlatform = text.match(regExMap[locale]['platform'])[1].replace("Platform", "").trim();
    	res.reservedTicket.ticketedSeat.seatNumber = text.match(regExMap[locale]['seat'])[1];
    	res.reservationFor.busNumber = text.match(regExMap[locale]['busLine'])[1].trim();
    	res.reservedTicket.ticketToken = 'qrCode:' + barcode.content;
    	let ticketCategory = text.match(regExMap[locale]['category'])
    	res.reservedTicket.name = ticketCategory ? ticketCategory[1] : undefined;
    	res.reservationFor.provider = { "@type": "Organization", name: text.match(regExMap[locale]['brand'])[1] }
	}

	ExtractorEngine.extractPrice(text, res);

    return res;
}
