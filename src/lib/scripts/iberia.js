/*
   SPDX-FileCopyrightText: 2017 Volker Krause <vkrause@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

function main(text) {
    var reservations = new Array();

    var bookingRef = text.match(/Booking code\n.*([0-9A-z]{6})\n/);
    if (!bookingRef)
        return reservations;

    var pos = bookingRef.index + bookingRef[0].length;
    while (true) {
        var firstLine = text.substr(pos).match(/From +([A-Z]{2})(\d{2,4}) +(\d{1,2}-\w{3}) +(\d{1,2}-\w{3}).*\n/);
        if (!firstLine)
            break;
        var index = firstLine.index + firstLine[0].length;

        var res = JsonLd.newFlightReservation();
        res.reservationNumber = bookingRef[1];
        res.reservationFor.flightNumber = firstLine[2];
        res.reservationFor.airline.iataCode = firstLine[1];

        var secondLine = text.substr(pos + index).match(/^ (\w+) \(([A-Z]{3})\) +(\d{2}:\d{2}) + (\d{2}:\d{2}) .*\n/);
        if (!secondLine)
            break;
        index += secondLine.index + secondLine[0].length;

        res.reservationFor.departureAirport.name = secondLine[1];
        res.reservationFor.departureAirport.iataCode = secondLine[2];
        res.reservationFor.departureTime = JsonLd.toDateTime(firstLine[3] + " " + secondLine[3], "dd-MMM hh:mm", "en");
        res.reservationFor.arrivalTime = JsonLd.toDateTime(firstLine[4] + " " + secondLine[4], "dd-MMM hh:mm", "en");

        var fourthLine = text.substr(pos + index).match(/^.*\n (\w+) \(([A-Z]{3})\) .*\n/);
        if (!fourthLine)
            break;
        index += fourthLine.index + fourthLine[0].length;

        res.reservationFor.arrivalAirport.name = fourthLine[1];
        res.reservationFor.arrivalAirport.iataCode = fourthLine[2];

        var opByLine = text.substr(pos + index).match(/^.*Operated by\n +: (.*)\n/);
        if (opByLine) {
            index += opByLine.index + opByLine[0].length;
            res.reservationFor.airline.name = opByLine[1];
        }

        reservations.push(res);
        if (index == 0)
            break;
        pos += index;
    }

    return reservations;
}

function extractBoardingPass(pdf, node, barcode)
{
    let res = barcode.result[0];
    const text = pdf.pages[barcode.location].text;
    const times = text.match(/\d\d \S+ (\d\d:\d\d) +(?:\d\d \S+)? (\d\d:\d\d)/);
    res.reservationFor.departureTime = JsonLd.toDateTime(times[1], 'hh:mm', 'en');
    res.reservationFor.arrivalTime = JsonLd.toDateTime(times[2], 'hh:mm', 'en');
    const boarding = text.match(/(?:SEAT|ASIENTO)\n *(\d\d:\d\d) (?:GRUPO (\S*))?/);
    res.reservationFor.boardingTime = JsonLd.toDateTime(boarding[1], 'hh:mm', 'en');
    res.boardingGroup = boarding[2];
    return res;
}

function extractPkPass(pass, node)
{
    let res = node.result[0];
    res.reservationFor.departureTime = JsonLd.toDateTime(pass.field['horaSalida'].value, 'hh:mm', 'es');
    res.reservationFor.arrivalTime = JsonLd.toDateTime(pass.field['horaLlegada'].value, 'hh:mm', 'es');
    res.boardingGroup = pass.field['group'].value
    return res;
}
