/*
   SPDX-FileCopyrightText: 2024 Luca Weiss <luca@lucaweiss.eu>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

function parsePdfTicket(pdf, node, triggerNode) {
    var res = parseBarcode(triggerNode.content);

    const text = pdf.pages[triggerNode.location].text;

    const stationMatch = text.match(/Nereden From +Nereye To\n +(.+?) {3,}(.+?) {3,}/);
    res.reservationFor.departureStation.name = stationMatch[1];
    res.reservationFor.arrivalStation.name = stationMatch[2];

    const passengerMatch = text.match(/Yolcu +Passenger\n +_+ +(.+?)\(([EK])\)/);
    res.underName.name = passengerMatch[1];
    // E = erkek/man, K = kadın/woman, no other option as of 2024
    const gender = passengerMatch[2];
    console.log("gender: " + gender);

    return res;
}

function parseBarcode(data) {
    var res = JsonLd.newTrainReservation();
    res.reservedTicket.ticketToken = "azteccode:" + data;
    console.log(data);
    const parts = data.split("$");
    console.log(parts);
    const identifier = parts[0];
    const unknown1 = parts[1];
    if (unknown1 != "6") { console.log("unexpected unknown1: " + unknown1) }
    const unknown2 = parts[2];
    if (unknown2 != "3") { console.log("unexpected unknown2: " + unknown2) }
    const unknown3 = parts[3];
    if (unknown3 != "0") { console.log("unexpected unknown3: " + unknown3) }

    res.reservedTicket.ticketNumber = parts[4];
    res.reservationNumber = parts[5]; // PNR
    res.reservationFor.departureTime = JsonLd.toDateTime(parts[6], "yyyyMMddhhmmss", "tr");

    const unknown7 = parts[7];
    console.log("unknown7: " + unknown7);

    const unknown8 = parts[8];
    console.log("unknown8: " + unknown8);

    const unknown9 = parts[9];
    console.log("unknown9: " + unknown9);

    const unknown10 = parts[10];
    console.log("unknown10: " + unknown10);

    res.reservationFor.trainNumber = parts[11];

    // https://github.com/rburaksaritas/tcdd-bilet-bulucu/blob/3ade97ace29ed5fb74c6d27961d8b5de9798567b/stations.json#L21
    const departureStationId = parts[12];
    console.log("departureStationId: " + departureStationId);
    res.reservationFor.departureStation.identifier = "tcdd:" + departureStationId;
    const arrivalStationId = parts[13];
    console.log("arrivalStationId: " + arrivalStationId);
    res.reservationFor.arrivalStation.identifier = "tcdd:" + arrivalStationId;

    const unknown14 = parts[14];
    if (unknown14 != "11750111100") { console.log("unexpected unknown14: " + unknown14) }

    res.reservedTicket.ticketedSeat.seatSection = parts[15];
    res.reservedTicket.ticketedSeat.seatNumber = parts[16];

    const unknown17 = parts[17];
    if (unknown17 != "1") { console.log("unexpected unknown17: " + unknown17) }

    res.reservedTicket.totalPrice = parts[18];
    res.reservedTicket.priceCurrency = 'TRY';

    // Non-discounted price
    const basePrice = parts[19];
    console.log("basePrice: " + basePrice);

    const purchaseDate = JsonLd.toDateTime(parts[20], "yyyyMMddhhmmss", "tr");
    console.log("purchaseDate: " + purchaseDate);

    const unknown21 = parts[21];
    if (unknown21 != "null") { console.log("unexpected unknown21: " + unknown21) }

    const unknown22 = parts[22];
    if (unknown22 != "null") { console.log("unexpected unknown22: " + unknown22) }

    const unknown23 = parts[23];
    if (unknown23 != "0") { console.log("unexpected unknown23: " + unknown23) }

    const unknown24 = parts[24];
    if (unknown24 != "") { console.log("unexpected unknown24: " + unknown24) }

    const unknown25 = parts[25];
    if (unknown25 != "1050") { console.log("unexpected unknown25: " + unknown25) }

    const unknown26 = parts[26];
    console.log("unknown26: " + unknown26);

    return res;
}
