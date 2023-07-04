/*
   SPDX-FileCopyrightText: 2023 Luca Weiss <luca@z3ntu.xyz>
   SPDX-License-Identifier: LGPL-2.0-or-later
*/

function parsePdfTicket(pdf, node, triggerNode) {
    // Parse the reservation data from the text in the pdf.
    // Note that in the English version most fields end with a colon, the Greek
    // version doesn't - as of 2023-07-04.
    const text = pdf.pages[triggerNode.location].text;
    let res = JsonLd.newBusReservation();

    // Potentially the "K12345 / E98765" number below the QR code would be a
    // better reservationNumber, but it's not clear enough for now.
    res.reservationNumber = triggerNode.content;
    res.reservedTicket.ticketToken = 'qrCode:' + triggerNode.content;

    const route = text.match(/(?:Route|Διαδρομή)[ ]+(.*)/);
    res.reservationFor.busNumber = route[1];

    const depTime = text.match(/(?:Boarding time:|Ώρα αναχώρησης)[ ]+(\d{4}-\d{2}-\d{2} \d{2}:\d{2})/);
    res.reservationFor.departureTime = JsonLd.toDateTime(depTime[1], 'yyyy-MM-dd hh:mm', 'gr');

    const dep = text.match(/(?:Boarding:|Επιβίβαση)[ ]+(.*)/);
    console.log(dep[1]);
    res.reservationFor.departureBusStop.name = dep[1];

    const arr = text.match(/(?:Disembark:|Αποβίβαση)[ ]+(.*)/);
    res.reservationFor.arrivalBusStop.name = arr[1];

    const name = text.match(/(?:Passenger name:|Όνομα επιβάτη)[ ]+(.*)/);
    res.underName.name = name[1];

    const seat = text.match(/(?:Seat number:|Θέση)[ ]+(\d+)/);
    res.reservedTicket.ticketedSeat.seatNumber = seat[1];

    const price = text.match(/(?:Price:|Τιμή)[ ]+(\d+\.\d{2})€/);
    res.totalPrice = price[1];

    return res;
}
