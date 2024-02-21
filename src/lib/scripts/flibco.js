/*
   SPDX-FileCopyrightText: 2024 Roberto Guido <info@madbob.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

function parsePdf(pdf, node, triggerNode) {
    const page = pdf.pages[0];
    const text = page.text;

    let res = JsonLd.newBusReservation();
    res.reservationNumber = text.match(/Ticket number: ([0-9]*)/)[1];

    let stations = text.match(/^From *To\n(.*?) {5,}(.*?)\n/m);
    res.reservationFor.departureBusStop.name = stations[1];
    res.reservationFor.arrivalBusStop.name = stations[2];

    const date = text.match(/^Valid On\n(.*?) {10,}/m)[1];
    res.reservationFor.departureTime = JsonLd.toDateTime(date + ' 00:01', 'MMM dd, yyyy hh:mm', 'en');

    // From the ticket itself:
    // Many departures every day. Your ticket is valid for the selected day until 04:00 the next day.
    let futureDay = new Date(JsonLd.toDateTime(date + ' 04:00', 'MMM dd, yyyy hh:mm', 'en'));
    futureDay.setDate(futureDay.getDate() + 1);
    res.reservationFor.arrivalTime = futureDay;

    const links = page.linksInRect(0, 0, 1, 1);
    res.modifyReservationUrl = links[0].url;
    res.reservationFor.departureBusStop.geo = JsonLd.toGeoCoordinates(links[1].url);
    res.reservationFor.arrivalBusStop.geo = JsonLd.toGeoCoordinates(links[2].url);

    return res;
}
