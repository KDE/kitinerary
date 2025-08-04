// SPDX-FileCopyrightText: 2025 David Pilarčík <meow@charliecat.space>
// SPDX-License-Identifier: LGPL-2.0-or-later

function parsePkPass(pass, node) {
    let res = node.result[0];
	res.underName = Object.assign(
		JsonLd.newObject('Person'), 
		{
			// The pass has a field of "name", it's label in the Sample is "E-mail". unsure if this is always the case
			// Fallback for the name if the label is "Nom"
			email: pass.field['name'].label == 'E-mail' ? pass.field['name'].value : undefined,
			name: pass.field['name'].label == 'Nom' ? pass.field['name'].value : undefined,
		}
	);
	// Yes, the field is called "type" in the pass. It's label is Séance du
	res.reservationFor.startDate = JsonLd.toDateTime(pass.field['type'].value, "dd/MM/yyyy HH:mm", "fr") // 28/10/2021 20:00
	res.reservationFor.duration = parseDuration(pass.field['duree'].value); // Durée: 2h10
	res.reservationFor.endDate = new Date(res.reservationFor.startDate)
	let durationP = parseDuration(pass.field['duree'].value, "object");
	res.reservationFor.endDate.setHours(res.reservationFor.endDate.getHours() + +durationP.hours);
	res.reservationFor.endDate.setMinutes(res.reservationFor.endDate.getMinutes() + +durationP.minutes);

	res.reservationFor.name = pass.field['movie'].value;
	res.reservationFor.location = Object.assign(
		JsonLd.newObject('Place'),
		{
			name: pass.field['cinema']?.value,

		}
	);

	res.reservedTicket.ticketedSeat = Object.assign(
		JsonLd.newObject('Seat'),
		{
			seatNumber: pass.field['places']?.value, // the sample includes 2 seats, separated by a comma
			seatSection: pass.field['salle']?.value, // I'm not sure where cinema halls should be
		}
	);

	res.reservationNumber = pass.barcodes[0].alternativeText
	res.bookingTime = JsonLd.toDateTime(pass.field['date'].value, "dd/MM/yyyy HH:mm", "fr") // Date de commande: 06/07/2020 17:51

	// in Sample: Moyen de paiement utilisé: 2 CB (16.6€)
	ExtractorEngine.extractPrice(pass.field["paiement"]?.value ?? "", res);

	// The sample also includes a NFC ticket token, which is not supported by Kitinerary nor KPkPass

	return res
}

// In sample: Durée: 2h10
function parseDuration(duration, type = "string") {
	const [hours, minutes] = duration.split('h');
	if (type === "string") {
		return `${hours.padStart(2, '0')}:${minutes.padStart(2, '0')}:00`;
	} else if (type === "object") {
		return { hours, minutes };
	}
}
