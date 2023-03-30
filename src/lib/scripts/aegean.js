/*
   SPDX-FileCopyrightText: 2022 Kai Uwe Broulik <kde@broulik.de>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

function main(pass, node)
{
    let res = node.result[0];

    const departureTime = JsonLd.toDateTime(pass.field["departuretime"].value, "hh:mm", "en");
    if (!isNaN(departureTime.getTime())) {
        res.reservationFor.departureTime = departureTime;
    }

    const arrivalTime = JsonLd.toDateTime(pass.field["arrivaltime"].value, "hh:mm", "en");
    if (!isNaN(arrivalTime.getTime())) {
        res.reservationFor.arrivalTime = arrivalTime;
    }

    return res;
}

function parseHtmlConfirmation(html)
{
    let reservations = [];
    const pnr = html.root.eval('//div[starts-with(@class, "bookCode")]')[0].recursiveContent;

    const flights = html.root.eval('//table[@class="flightBoundLineRecap" or @class="detailContainer"]');
    let date = "";
    for (flight of flights) {
        if (flight.attribute('class') == 'flightBoundLineRecap') {
            date = flight.eval('.//strong')[0].content.match(/\S+ (.*)/)[1];
            continue;
        }
        let res = JsonLd.newFlightReservation();
        res.reservationNumber = pnr;

        const origin = flight.eval('.//*[@class="originDateAirport"]')[0].recursiveContent.match(/(\d\d:\d\d)\s+(.*)/);
        res.reservationFor.departureAirport.name = origin[2];
        res.reservationFor.departureTime = JsonLd.toDateTime(date + ' ' + origin[1], 'dd.MMM hh:mm', ['en', 'de']);
        const destination = flight.eval('.//*[@class="destinationDateAirport"]')[0].recursiveContent.match(/(\d\d:\d\d)\s+(.*)/);
        res.reservationFor.arrivalAirport.name = destination[2];
        res.reservationFor.arrivalTime = JsonLd.toDateTime(date + ' ' + destination[1], 'dd.MMM hh:mm', ['en', 'de']);
        const airline = flight.eval('.//div[@class="aircraftAndAirline"]/div')[0].content.match(/([A-Z0-9]{2})(\d{1,4})/);
        res.reservationFor.airline.iataCode = airline[1];
        res.reservationFor.flightNumber = airline[2];

        reservations.push(res);
    }

    return reservations;
}
