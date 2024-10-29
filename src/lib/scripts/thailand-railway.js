// SPDX-FileCopyrightText: 2024 Volker Krause <vkrause@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

function extractPdfTicket(pdf, node, barcode) {
    const text = pdf.pages[barcode.location].text;

    // TODO multi passenger support?
    let res = JsonLd.newTrainReservation();
    res.reservationNumber = barcode.content.match(/\/(\d+)\//)[1];
    res.reservedTicket.ticketToken = 'qrCode:' + barcode.content;
    res.underName.name = text.match(/Passenger Name : (\S.*?\S)  /)[1];
    res.reservationFor.trainNumber = text.match(/Train : (\S.*?)\s\s/)[1];
    const trip = text.match(/Origin : (.*?)\s\s+Destination : (.*)/);
    res.reservationFor.departureStation.name = trip[1];
    res.reservationFor.arrivalStation.name = trip[2];
    const times = text.match(/(\d\d\/\d\d\/\d{4})\s+(\d\d:\d\d)\s+(\d\d:\d\d)\s+(\S.*?)\s-\s.*(\S.*)-(\S.*?)\s\s/);
    res.reservationFor.departureTime = JsonLd.toDateTime(times[1] + times[2], "dd/MM/yyyyhh:mm", "en");
    res.reservationFor.arrivalTime = JsonLd.toDateTime(times[1] + times[3], "dd/MM/yyyyhh:mm", "en");
    res.reservedTicket.ticketedSeat.seatingType = times[4];
    res.reservedTicket.ticketedSeat.seatSection = times[5];
    res.reservedTicket.ticketedSeat.seatNumber = times[6];
    return res;
}
