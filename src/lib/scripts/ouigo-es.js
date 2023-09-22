/*
   SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
   SPDX-License-Identifier: LGPL-2.0-or-later
*/

// barcode format
// 13x ticket number
// 5x UIC issuer/carrier code
// 5x train number
// 15x departure time (dd/MM/yyyyhh:mm)
// 7x departure UIC station code
// 7x arrival UIC station code
// 3x coach number
// 3x seat number
// more stuff - signature?
function parseBarcode(content) {
    let res = JsonLd.newTrainReservation();
    res.reservedTicket.ticketToken = 'azteccode:' + content;
    res.reservedTicket.ticketNumber = content.substr(0, 13);
    res.reservationFor.provider.identifier = 'uic:' + content.substr(13, 5);
    res.reservationFor.trainNumber = content.substr(18, 5);
    res.reservationFor.departureTime = JsonLd.toDateTime(content.substr(23, 15), 'dd/MM/yyyyhh:mm', 'es');
    res.reservationFor.departureStation.identifier = 'uic:71' + content.substr(40, 5);
    res.reservationFor.departureStation.name =  content.substr(38, 7);
    res.reservationFor.arrivalStation.identifier = 'uic:71' + content.substr(47, 5);
    res.reservationFor.arrivalStation.name = content.substr(45, 7);
    res.reservedTicket.ticketedSeat.seatSection = content.substr(52, 3);
    res.reservedTicket.ticketedSeat.seatNumber = content.substr(55, 3);
    return res;
}

function parsePdf(pdf, node, triggerNode) {
    const page = pdf.pages[triggerNode.location];
    let res = triggerNode.result[0];

    const topLeft = page.textInRect(0.0, 0.0, 0.5, 0.5);
    const stations = topLeft.match(/\n(.*)\n\d\d:\d\d\n(.*)\n(\d\d:\d\d)/);
    if (stations) {
        res.reservationFor.departureStation.name = stations[1];
        res.reservationFor.arrivalStation.name = stations[2];
        res.reservationFor.arrivalTime = JsonLd.toDateTime(stations[3], "hh:mm", "es");
    } else {
        const stationsV2 = topLeft.match(/\n\d\d:\d\d\n(.*)\n(\d\d:\d\d)\n(.*)/);
        res.reservationFor.departureStation.name = stationsV2[1];
        res.reservationFor.arrivalStation.name = stationsV2[3];
        res.reservationFor.arrivalTime = JsonLd.toDateTime(stationsV2[2], "hh:mm", "es");
    }

    const topRight = page.textInRect(0.5, 0.0, 1.0, 0.5);
    res.underName.name = topRight.match(/^(.*)\n/)[1];
    res.reservationNumber = topRight.match(/  +([A-Z0-9]{6})\n/)[1];
    return res;
}
