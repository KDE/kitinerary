// SPDX-FileCopyrightText: 2025 Volker Krause <vkrause@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

function extractEvent(ev) {
    let res = JsonLd.newFlightReservation();
    res.reservationNumber = ev.description.match(/Checkin reference: (.*)\n/)[1];
    res.underName.name = ev.description.match(/Travelers: (.*)\n/)[1];
    res.reservationFor.departureTime = JsonLd.readQDateTime(ev, "dtStart");
    res.reservationFor.arrivalTime = JsonLd.readQDateTime(ev, "dtEnd");
    res.reservationFor.departureAirport.iataCode = ev.location;
    const arr = ev.summary.match(/Flight to (.*) \((.*)\)/);
    res.reservationFor.arrivalAirport.name = arr[1];
    res.reservationFor.arrivalAirport.iataCode = arr[2];
    ExtractorEngine.extractPrice(ev.description, res);
    return res;
}
