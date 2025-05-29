/*
    SPDX-FileCopyrightText: 2025 Johannes Krattenmacher <git.noreply@krateng.ch>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

function parsePdf(pdf, node) {
    var res = JsonLd.newBoatReservation();
    var pdfText = pdf.text;
    res.reservationNumber = pdfText.match(/Booking number (\d{5,10})/)[1];

    // use the more specific info for departure
    var info = pdfText.match(/Port information and check-in times\n(.*):(.*)\nAddress\n(.*), (\d+) (.*)\nGPS: N\/lat\s*(\d{1,3})°\s*(\d{1,2}\.\d+)',\s*E\/lon\s*(\d{1,3})°\s*(\d{1,2}\.\d+)'/);
    // viking lines is always N E

    res.reservationFor.departureBoatTerminal.name = info[2];
    res.reservationFor.departureBoatTerminal.address.streetAddress = info[3];
    res.reservationFor.departureBoatTerminal.address.addressLocality = info[5];
    res.reservationFor.departureBoatTerminal.address.postalCode = info[4];
    res.reservationFor.departureBoatTerminal.geo.latitude = parseInt(info[6], 10) + (parseFloat(info[7])/60);
    res.reservationFor.departureBoatTerminal.geo.longitude = parseInt(info[8], 10) + (parseFloat(info[9])/60);

    var shortinfo = pdfText.match(/Departure[^\S\r\n]*(.*) (.{3}) (\d{1,2}) (.+) (\d{4}) at (\d{1,2}):(\d{2})\nArrival[^\S\r\n]*(.*) (.{3}) (\d{1,2}) (.+) (\d{4}) at (\d{1,2}):(\d{2})/);
    var depart = shortinfo[3] + " " + shortinfo[4] + " " + shortinfo[5] + " " + shortinfo[6] + ":" + shortinfo[7];
    var arrive = shortinfo[10] + " " + shortinfo[11] + " " + shortinfo[12] + " " + shortinfo[13] + ":" + shortinfo[14];

    res.reservationFor.arrivalBoatTerminal.address.addressLocality = shortinfo[8];
    // terminal name needed for validation? this just uses the city for now
    res.reservationFor.arrivalBoatTerminal.name = shortinfo[8];
    res.reservationFor.departureTime = JsonLd.toDateTime(depart, 'd MMMM yyyy HH:mm', 'en');
    res.reservationFor.arrivalTime = JsonLd.toDateTime(arrive, 'd MMMM yyyy HH:mm', 'en');

    return res;
}


