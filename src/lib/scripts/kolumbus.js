/*
   SPDX-FileCopyrightText: 2025 Volker Krause <vkrause@kde.org>
   SPDX-License-Identifier: LGPL-2.0-or-later
*/

function extractPdf(pdf) {
    const text = pdf.text;
    console.log(text);
    let res = JsonLd.newBoatReservation();
    res.reservationNumber = text.match(/Booking no: (\d+)/)[1];
    res.underName.name = text.match(/Traveller: +(.*)/)[1];
    const trip = text.match(/(\S.*\d, \d{4}).*\n\s*(\d\d:\d\d) (\S.*\S)  +(\d\d:\d\d) (\S.*?\S)  +(\S+)\n/);
    console.log(trip);
    res.reservationFor.departureBoatTerminal.name = trip[3];
    res.reservationFor.departureTime = JsonLd.toDateTime(trip[1] + trip[2], 'MMMM dd, yyyyhh:mm', 'en');
    res.reservationFor.arrivalBoatTerminal.name = trip[5];
    res.reservationFor.arrivalTime = JsonLd.toDateTime(trip[1] + trip[4], 'MMMM dd, yyyyhh:mm', 'en');
    return res;
}
