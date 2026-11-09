/*
   SPDX-FileCopyrightText: 2017 Volker Krause <vkrause@kde.org>
   SPDX-FileCopyrightText: 2018 Daniel Vrátil <dvratil@kde.org>
   SPDX-FileCopyrightText: 2026 David Pilarcik <meow@charliecat.space>
   
   SPDX-License-Identifier: LGPL-2.0-or-later
*/

function parsePkPass(pass, node) {
	let res = JsonLd.newBusReservation();

	if (node.result.length > 0 && node.result[0].reservedTicket)
		res.reservedTicket = node.result[0].reservedTicket;
	else {
		res.reservedTicket.ticketToken = "qrCode:" + pass.barcodes[0].message;
	}
	res.reservationFor.departureBusStop.name = pass.field["from"].label;
    res.reservationFor.departureTime = JsonLd.toDateTime(pass.field["departure_date"].value + pass.field["boarding_time"].value, "MMMddhh:mm", "en");
    res.reservationFor.departurePlatform = pass.field["platform"].value;
    res.reservationFor.arrivalBusStop.name = pass.field["to"].label;
    res.reservationNumber = pass.field["booking_number"].value;
    res.underName.name = pass.field["passenger"].value;
    res.reservedTicket.ticketedSeat = {
        "@type": "Seat",
        seatNumber: pass.field["seat_number"].value
    };
    res.reservationFor.busName = pass.field["route"].value;
    res.reservationFor.provider.name = pass.field["operated_by"].value;
    return res;
}