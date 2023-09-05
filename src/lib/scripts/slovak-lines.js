/*
   SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
   SPDX-License-Identifier: LGPL-2.0-or-later
*/

function extractPass(pass, node) {
    let res = node.result[0];
    res.reservationFor = JsonLd.newObject("BusTrip");
    res.reservationFor.departureTime =  JsonLd.toDateTime(pass.field["date"].value + ' ' + pass.field["origin"].value, "dd.MMM hh:mm", "en");
    res.reservationFor.departureBusStop = {
        '@type': 'BusStation',
        name: pass.field["origin_long"].value,
        address: {
            '@type': 'PostalAddress',
            addressLocality: pass.field["origin"].label
        },
        geo: {
            '@type': 'GeoCoordinates',
            latitude: pass.locations[0].latitude,
            longitude: pass.locations[0].longitude
        }
    }
    res.reservationFor.departurePlatform = pass.field["platform"].value;
    res.reservationFor.arrivalTime =  JsonLd.toDateTime(pass.field["date"].value + ' ' + pass.field["destination"].value, "dd.MMM hh:mm", "en");
    res.reservationFor.arrivalBusStop = {
        '@type': 'BusStation',
        name: pass.field["destination_long"].value,
        address: {
            '@type': 'PostalAddress',
            addressLocality: pass.field["destination"].label
        }
    };
    res.reservedTicket.ticketedSeat = {
        '@type': 'Seat',
        seatNumber: pass.field["tariff"].value.match(/\/(.*)/)[1]
    };
    return res;
}

function extractTicket(pdf, node, barcode) {
    const text = pdf.pages[barcode.location].text;
    let res = JsonLd.newBusReservation();
    res.underName.name = text.match(/^.*?: +(.*?)  /)[1];
    res.reservationNumber = barcode.content;
    const dep = text.match(/Departure: +(.*) \S+, (\d\d\.\d\d\.\d{4} \d\d:\d\d)/);
    res.reservationFor.departureBusStop.name = dep[1];
    res.reservationFor.departureTime = JsonLd.toDateTime(dep[2], 'dd.MM.yyyy hh:mm', 'en');
    const arr = text.match(/Arrival: +(.*) \S+, (\d\d\.\d\d\.\d{4} \d\d:\d\d)/);
    res.reservationFor.arrivalBusStop.name = arr[1];
    res.reservationFor.arrivalTime = JsonLd.toDateTime(arr[2], 'dd.MM.yyyy hh:mm', 'en');
    res.reservationFor.departurePlatform = text.match(/Platform: +(\S.*)/)[1];
    res.reservedTicket.ticketedSeat.seatNumber = text.match(/Seat: +(\S.*)/)[1];
    res.reservationFor.busNumber = text.match(/Bus line no: +(.*)  /)[1];
    res.reservedTicket.ticketToken = 'qrCode:' + barcode.content;
    ExtractorEngine.extractPrice(text, res);
    return res;
}
