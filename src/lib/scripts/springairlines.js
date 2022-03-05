/*
   SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
   SPDX-License-Identifier: LGPL-2.0-or-later
*/

function parseConfirmation(content) {
    var reservations = [];
    const text = content.pages[0].text;
    const ref = text.match(/Reference.\s*([A-Z0-9]{6})/)[1];
    var idx = 0;
    while (true) {
        const leg = text.substr(idx).match(/Flight Time\n\s*(\d\d:\d\d)\s*(\d\d:\d\d)\n\s*(\d\d\d\d\/\d\d\/\d\d)\s*\(.*\)\s+([A-Z0-9]{2}\d{1,4}).*\n\s*(.*?)\s\s+(.*)/);
        if (!leg) {
            break;
        }
        idx += leg.index + leg[0].length;

        var flight = JsonLd.newFlightReservation();
        flight.reservationNumber = ref;
        flight.reservationFor.departureAirport.name = leg[5];
        flight.reservationFor.departureTime = JsonLd.toDateTime(leg[3] + leg[1], "yyyy/MM/ddhh:mm", "jp");
        flight.reservationFor.arrivalAirport.name = leg[6];
        flight.reservationFor.arrivalTime = JsonLd.toDateTime(leg[3] + leg[2], "yyyy/MM/ddhh:mm", "jp");
        flight.reservationFor.flightNumber = leg[4];

        // TODO handle multiple passengers, once we have a sample for that
        const pas = text.substr(idx).match(/\s*(.*)\/(.*?)  +.* (\d{1,2}[A-K]).*\n/);
        idx += leg.index + leg[0].length;

        flight.underName.familyName = pas[1];
        flight.underName.givenName = pas[2];
        flight.airplaneSeat = pas[3];

        reservations.push(flight);
    }
    return reservations;
}
