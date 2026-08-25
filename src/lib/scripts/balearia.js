// SPDX-FileCopyrightText: 2026 David Pilarčík <meow@charliecat.space>
// SPDX-License-Identifier: LGPL-2.0-or-later

const UNLOCODEsToCityServedName = {
	// see https://unlocode.unece.org/directory & https://www.balearia.com/es/destinos
	"DZALG": "Argel",
	"DZGHZ": "Ghazaouet",
	"DZMOS": "Mostaganem",
	"DZORN": "Orán",
	"BSBIM": "Bimini",
	"BSFPO": "Grand Bahama",
	"USFLL": "Fort Lauderdale",
	"ESALG": "Algeciras",
	"ESLEI": "Almería",
	"ESBCN": "Barcelona",
	"ESCAD": "Cádiz",
	"ESCEU": "Ceuta",
	"ESDEN": "Dénia",
	"ESFLD": "Formentera",
	"ESFUE": "Fuerteventura",
	"ESIBZ": "Ibiza",
	"ESSSG": "La Gomera",
	"ESACE": "Lanzarote",
	"ESSPC": "La Palma",
	"ESLPA": "Las Palmas de Gran Canaria",
	"ESAGP": "Málaga",
	"ESPMI": "Mallorca",
	"ESMLN": "Melilla",
	"ESMAH": "Menorca",
	"ESMOT": "Motril",
	"ESTRF": "Tarifa",
	"ESSCT": "Tenerife",
	"ESVLC": "València",
	"MAAHU": "Al Hoceima",
	"MANDR": "Nador",
	"MAPTM": "Tánger Med",
	"MATNG": "Tánger Ville"
}

function parseBoardingPass(contents) {
	// example:
	// `0NTEmbarque019500000000%000ESTRFMATNG957957dd/MM/yyyy hh:mm199000XX1234567DAVID PILARCIK                                                                                      `
	//  \----------/\-------------/\---/\---/\----/\--------------/\----/\-------/\---------------... 175
	//   NTEmbarque  N. Billete     Dep  Arr  unk1  departure date  unk2  ID SNR   Passanger Name
	//
	// unk1 & unk2 look to be static from paper ticket samples & what was found online
	// the ticket itself includes a PNR-like value "Localizador" used to (together with identity document snr) edit the booking on the website, but it is not included in the ticket token contents
	// in my opinion the N.T. Embarque represents a value like the IATA's BCBP Pax Sequence Number
	let [_, NTEmbarque, NBillete, DepPort, ArrPort, departureDateTime, IdentityDocuemntSRN, passangerName]
		= /(\d{12})(\d{15})([A-Z]{5})([A-Z]{5}).{6}(.{16}).{6}(.{9})(.*)/gm.exec(contents);

	const res = JsonLd.newBoatReservation();

	res.reservationFor.departureBoatTerminal.unlocode = DepPort;
	res.reservationFor.departureBoatTerminal.name = UNLOCODEsToCityServedName[DepPort] || DepPort;
	res.reservationFor.departureBoatTerminal.address.addressCountry = getCountryCodeFromUNLOCODE(DepPort);
	res.reservationFor.arrivalBoatTerminal.unlocode = ArrPort;
	res.reservationFor.arrivalBoatTerminal.name = UNLOCODEsToCityServedName[ArrPort] || ArrPort;
	res.reservationFor.arrivalBoatTerminal.address.addressCountry = getCountryCodeFromUNLOCODE(ArrPort);
	res.reservationFor.departureTime = JsonLd.toDateTime(departureDateTime, 'dd/MM/yyyy hh:mm', 'es');
	res.reservedTicket.ticketToken = 'pdf417:' + contents;
	res.reservedTicket.ticketNumber = NBillete;
	res.underName.name = passangerName
	res.underName.passportNumber = IdentityDocuemntSRN
	res.passengerSequenceNumber = NTEmbarque

	return res
}

function parsePdf(pdf, node, barcode) {
	const page = pdf.pages[barcode.location];
	const res = node.result[0];

	// unable to get pdf tickets found online be read

	res.reservationFor.boatName = page.textInRect(0.5, 0.2, 0.7, 0.3)
	// Acomodación/Accomodation field
	res.description = page.textInRect(0.1, 0.3, 0.4, 0.4)
	// Edad/Age field
	res.reservedTicket.name = page.textInRect(0.4, 0.3, 0.5, 0.3)
	// Sexo/Gender field
	res.underName.gender = page.textInRect(0.5, 0.3, 0.6, 0.3)
	// Localizador/Booking Ref field
	res.reservationNumber = page.textInRect(0.1, 0.2, 0.2, 0.2)

	return null
}

/**
 * @param {string} unlocode
 * @returns {string} country code
 */
function getCountryCodeFromUNLOCODE(unlocode) {
	return unlocode.substr(0, 2);
}

