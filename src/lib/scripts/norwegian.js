/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

function parsePage(page)
{
    var res = JsonLd.newFlightReservation();

    var pnr = page.text.match(/\nBooking reference.*(.{6})\n/);
    if (!pnr)
        return null;
    res.reservationNumber = pnr[1];

    var name = page.text.match(/\nPassenger\s+(?:\S+)? ([A-Z ]+)\/(.+)\n/);
    res.underName.familyName = name[1];
    res.underName.givenName = name[2];

    var flight = page.text.match(/\nFlight.*?(\S{2})(\d{1,4}) - (.*)\n/);
    res.reservationFor.airline.iataCode = flight[1];
    res.reservationFor.flightNumber = flight[2];

    var dep = page.text.match(/\nDeparture.*(\d{2}:\d{2}) (.*) \(([A-Z]{3})\)(.*)\n/);
    res.reservationFor.departureAirport.name = dep[2];
    res.reservationFor.departureAirport.iataCode = dep[3];
    res.reservationFor.departureTerminal = dep[4];
    res.reservationFor.departureTime = JsonLd.toDateTime(flight[3] + dep[1], "dd MMM yyyyhh:mm", "en");
    if (isNaN(res.reservationFor.departureTime.getTime()))
        res.reservationFor.departureTime = JsonLd.toDateTime(flight[3] + dep[1], "yyyy MMM ddhh:mm", "en");

    var arr = page.text.match(/\Arrival.*(\d{2}:\d{2}) (.*) \(([A-Z]{3})\)(.*)\n/);
    res.reservationFor.arrivalAirport.name = arr[2];
    res.reservationFor.arrivalAirport.iataCode = arr[3];
    res.reservationFor.arrivalTerminal = arr[4];
    res.reservationFor.arrivalTime = JsonLd.toDateTime(flight[3] + arr[1], "dd MMM yyyyhh:mm", "en");
    if (isNaN(res.reservationFor.arrivalTime.getTime()))
        res.reservationFor.arrivalTime= JsonLd.toDateTime(flight[3] + arr[1], "yyyy MMM ddhh:mm", "en");

    var seat = page.text.match(/Seat.* (\d{1,3}[A-J]) .*\n/);
    if (seat) {
        res.airplaneSeat = seat[1];
    }

    var group = page.text.match(/Boarding group.* (\w)\n/);
    if (group) {
        res.boardingGroup = group[1];
    }

    if (Context.barcode) {
        res.reservedTicket.ticketToken = "azteccode:" + Context.barcode;
    }

    return res;
}

function extractPdf(pdf) {
    if (Context.pdfPageNumber >= 0) {
        return parsePage(pdf.pages[Context.pdfPageNumber]);
    }
    var results = new Array();
    var pages = pdf.pages;
    for (var i = 0; i < pages.length; ++i) {
        results.push(parsePage(pages[i]));
    }
    return results;
}
