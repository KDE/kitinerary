/*
   SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
   SPDX-License-Identifier: LGPL-2.0-or-later
*/

function parseFlightEvent(event) {
    let res = JsonLd.newFlightReservation();
    const summary = event.summary.match(/([A-Z0-9]{2}) +(\d{1,4}) ([A-Z]{3})\/([A-Z]{3})/);
    res.reservationFor.departureAirport.iataCode = summary[3];
    res.reservationFor.arrivalAirport.iataCode = summary[4];
    res.reservationFor.departureTime = JsonLd.readQDateTime(event, 'dtStart');
    res.reservationFor.arrivalTime = JsonLd.readQDateTime(event, 'dtEnd');
    res.reservationFor.airline.iataCode = summary[1];
    res.reservationFor.flightNumber = summary[2];
    res.reservationFor.airline.name = event.description.match(/(.*), flight/)[1];
    const dep = event.description.match(/Depart:.* - (.*) - .*\.(?: Terminal (.*))?/);
    res.reservationFor.departureAirport.name = dep[1];
    res.reservationFor.departureTerminal = dep[2];
    const arr  = event.description.match(/Arrival:.* - (.*) - .*\.(?: Terminal (.*))?/);
    res.reservationFor.arrivalAirport.name = arr[1];
    res.reservationFor.arrivalTerminal = arr[2];
    res.reservationNumber = event.description.match(/Airline reference: (.*)/)[1];
    return res;
}

function parseHotelEvent(event) {
    let res = JsonLd.newLodgingReservation();
    res.checkinTime = JsonLd.readQDateTime(event, 'dtStart');
    res.checkoutTime = JsonLd.readQDateTime(event, 'dtEnd');
    res.reservationFor.name = event.location.match(/Staying at (.*) - /)[1];
    const addr = event.description.match(/Hotel information\n(.*)\n(.*),(.*),(.*)\nPhonenumber: (.*)/);
    res.reservationFor.address.streetAddress = addr[1];
    res.reservationFor.address.addressLocality = addr[2];
    res.reservationFor.address.postalCode = addr[4].match(/[A-Z]{2}/) ? addr[3] : addr[4];
    res.reservationFor.address.addressCountry = addr[4].match(/[A-Z]{2}/) ? addr[4] : addr[3];
    res.reservationFor.telephone = '+' + addr[5];
    console.log(event);
    return res;
}
