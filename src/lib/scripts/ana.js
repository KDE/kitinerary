/*
   SPDX-FileCopyrightText: 2024 Volker Krause <vkrause@kde.org>
   SPDX-License-Identifier: LGPL-2.0-or-later
*/

function extractPdfEticket(pdf, node, barcode) {
    const text = pdf.pages[barcode.location].text;
    const pnr = barcode.content.match(/(\d{13})\/([A-Z0-9]{6})/);
    const pas = text.match(/(?:PASSENGER :|NAME) *(\S.*\S)\/(\S.*\S)\n/);
    let reservations = [];
    let idx = 0;
    while (true) {
        const leg = text.substr(idx).match(/\] (\S.*?\S) (?: +(\S+)  )? +([A-Z-0-9]{2})(\d{1,4}) +(\d{2}[A-Z]{3}\d{2}) +[A-Z]{3} +(\d{4}).*\n.*\n+ *(\S.*?\S) (?: +(\S+)  )? +(\d{2}[A-K])? +(\d{2}[A-Z]{3}\d{2}) +[A-Z]{3} +(\d{4}) *(\S.*\S)/);
        if (!leg)
            break;
        idx += leg.index + leg[0].length;

        let res = JsonLd.newFlightReservation();
        res.reservedTicket.ticketNumber = pnr[1];
        res.reservationNumber = pnr[2];
        res.reservationFor.departureAirport.name = leg[1];
        res.reservationFor.departureTerminal = leg[2];
        res.reservationFor.airline.iataCode = leg[3];
        res.reservationFor.flightNumber = leg[4];
        res.reservationFor.departureTime = JsonLd.toDateTime(leg[5] + leg[6], "ddMMMyyhhmm", "en");
        res.reservationFor.arrivalAirport.name = leg[7];
        res.reservationFor.arrivalTerminal = leg[8];
        res.airplaneSeat = leg[9]
        res.reservationFor.arrivalTime = JsonLd.toDateTime(leg[10] + leg[11], "ddMMMyyhhmm", "en");
        res.reservationFor.airline.name = leg[12];
        res.underName.familyName = pas[1];
        res.underName.givenName = pas[2];
        reservations.push(res);
    }

    return reservations;
}
