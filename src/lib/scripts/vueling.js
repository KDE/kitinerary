/*
   SPDX-FileCopyrightText: 2017 Volker Krause <vkrause@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

function parseHtmlBooking(doc) {
    var reservations = new Array();

    var bookingRef = doc.eval("//td[@class=\"vuelo_confirmado_header\"]")[0].firstChild.content;
    if (!bookingRef)
        return null;

    var lang = doc.eval('//a[@class="copyright-text"]')[0].attribute("href").match(/\/([a-z]{2})$/)[1];

    var elems = doc.eval("//td[@class=\"vuelo_confirmado_card__subheader\"]");
    for (var i = 0; i < elems.length; ++i) {
        var elem = elems[i];
        var detailsRoot = elem.parent.nextSibling;

        var res = JsonLd.newFlightReservation();
        res.reservationNumber = bookingRef;

        var airportName = detailsRoot.eval(".//td[@class=\"vuelo_confirmado_card_details--city salida\"]")[0];
        var iataCode = detailsRoot.eval(".//td[@class=\"vuelo_confirmado_card_details--iata\"]")[0];
        res.reservationFor.departureAirport.iataCode = iataCode.content;
        res.reservationFor.departureAirport.name = airportName.content;
        res.reservationFor.arrivalAirport.iataCode = iataCode.nextSibling.content;
        res.reservationFor.arrivalAirport.name = airportName.nextSibling.content;

        var time = detailsRoot.eval(".//td[@class=\"vuelo_confirmado_card_details--time\"]")[0];
        res.reservationFor.departureTime = JsonLd.toDateTime(elem.content + ' ' + time.content.replace('h', ''), "dddd, dd MMMM yyyy HH:mm", lang);
        res.reservationFor.arrivalTime = JsonLd.toDateTime(elem.content + ' ' + time.nextSibling.content.replace('h', ''), "dddd, dd MMMM yyyy HH:mm", lang);

        var flightNum = detailsRoot.recursiveContent.match(/\s([A-Z0-9]{2})(\d{1,4})\b/);
        res.reservationFor.flightNumber = flightNum[2];
        res.reservationFor.airline.iataCode = flightNum[1];

        reservations.push(res);
    }

    return reservations;
}

function parsePdfBoardingPass(pdf, node, triggerNode) {
    let res = triggerNode.result[0];
    const page = pdf.pages[triggerNode.location];
    const topRight = page.textInRect(0.5, 0.0, 1.0, 0.5);
    const times = topRight.match(/(\d{2}:\d{2}) (?:H|Uhr) +(\d{2}:\d{2}) (?:H|Uhr)/);
    res.reservationFor.departureTime = JsonLd.toDateTime(times[1], "hh:mm", "en");
    res.reservationFor.arrivalTime = JsonLd.toDateTime(times[2], "hh:mm", "en");
    const topLeft = page.textInRect(0.0, 0.0, 0.5, 1.0);
    const grp = topLeft.match(/(?:Group|Grupo|Gruppe) +(\d)/i);
    if (grp) {
        res.boardingGroup = grp[1];
    }
    return res;
}
