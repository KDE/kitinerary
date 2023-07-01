/*
   SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
   SPDX-License-Identifier: LGPL-2.0-or-later
*/

function extractPdf(pdf) {
    const text = pdf.text;
    let res = JsonLd.newBoatReservation();
    res.reservationNumber = text.match(/Booking reference: (\S+)/)[1];
    res.underName.name = text.match(/Responsible: (.*)/)[1];
    const trip = text.match(/Departure: (.*) \S+ (\d\d\.\d\d\.\d{4} \d\d:\d\d) Arrival: (.*) \S+ (\d\d\.\d\d\.\d{4} \d\d:\d\d)/);
    res.reservationFor.departureBoatTerminal.name = trip[1];
    res.reservationFor.departureTime = JsonLd.toDateTime(trip[2], 'dd.MM.yyyy hh:mm', 'en');
    res.reservationFor.arrivalBoatTerminal.name = trip[3];
    res.reservationFor.arrivalTime = JsonLd.toDateTime(trip[4], 'dd.MM.yyyy hh:mm', 'en');
    return res;
}
