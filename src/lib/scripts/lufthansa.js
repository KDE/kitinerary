/*
   SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
   SPDX-License-Identifier: LGPL-2.0-or-later
*/

function extractEvent(ev) {
    let res = JsonLd.newFlightReservation();

    // force UTC, otherwise we lose the timezone due to JS converting to the local TZ
    res.reservationFor.departureTime = ev.dtStart.toJSON();
    res.reservationFor.arrivalTime = ev.dtEnd.toJSON();

    const flight = ev.description.match(/Flight no: (\S{2}) (\d+)/);
    res.reservationFor.airline.iataCode = flight[1];
    res.reservationFor.flightNumber = flight[2];
    res.reservationFor.airline.name = ev.description.match(/operated by: (.*)/)[1];

    const dep = ev.location.match(/from (.*\(([A-Z]{3})\).*)/);
    res.reservationFor.departureAirport.name = dep[1];
    res.reservationFor.departureAirport.iataCode = dep[2];

    const arr = ev.summary.match(/to (.*\(([A-Z]{3})\).*)/);
    res.reservationFor.arrivalAirport.name = arr[1];
    res.reservationFor.arrivalAirport.iataCode = arr[2];

    res.reservationNumber = ev.description.match(/Reservation code: (.*)/)[1];
    return res;
}

function extractBoardingPass(iata, node, pdfNode) {
    let res = node.result[0];
    const page = pdfNode.content.pages[node.location];

    // new? 2025 multi-column layout
    const rightCol = page.textInRect(0.80, 0.0, 1.0, 1.0);
    let boarding = rightCol.match(/Group\n(.*)\n *Boarding\n *(\d\d:\d\d)\n/);
    if (boarding) {
        res.boardingGroup = boarding[1];
        res.reservationFor.boardingTime = JsonLd.toDateTime(boarding[2], 'hh:mm', 'en');

        const leftCol = page.textInRect(0.0, 0.0, 0.65, 1.0);
        const times = leftCol.match(/(\d\d:\d\d)  +(\d\d:\d\d)\n/);
        res.reservationFor.departureTime = JsonLd.toDateTime(times[1], 'hh:mm', 'en');
        res.reservationFor.arrivalTime = JsonLd.toDateTime(times[2], 'hh:mm', 'en');

        const midCol = page.textInRect(0.65, 0.0, 0.8, 1.0);
        const dep = midCol.match(/Terminal\n *(.*)\n *Gate\n *(.*)\n/);
        if (dep) {
            res.reservationFor.departureTerminal = dep[1];
            res.reservationFor.departureGate = dep[2];
        }

        return res;
    }

    // pre-2025 single column layout
    const text = page.text;
    boarding = text.match(/(\d\d:\d\d) (?:GROUP (\S+))?  +\d+[A-Z]/);
    if (!boarding)
        boarding = text.match(/(\d\d:\d\d) +\d\d:\d\d/);
    res.reservationFor.boardingTime = JsonLd.toDateTime(boarding[1], 'hh:mm', 'en');
    res.boardingGroup = boarding[2];
    return res;
}
