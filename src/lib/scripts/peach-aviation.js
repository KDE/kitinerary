/*
   SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
   SPDX-License-Identifier: LGPL-2.0-or-later
*/

function parseReservation(content) {
    const ref = content.match(/Booking Reference\n(\S{6})/)[1];

    // TODO multiple legs?
    const leg = content.match(/(\S{2}\d{1,4})】[\s\S]*?├ (.*)　(.*)\n├ (\d{4}\/\d{2}\/\d{2}).*(\d{2}:\d{2})\n｜ ↓\n├ (.*)　(.*)\n└ (\d{4}\/\d{2}\/\d{2}).*(\d{2}:\d{2})/);

    var res = JsonLd.newFlightReservation();
    res.reservationNumber = ref;
    res.reservationFor.flightNumber = leg[1];
    res.reservationFor.departureAirport.name = leg[2];
    res.reservationFor.departureTerminal = leg[3];
    res.reservationFor.departureTime = JsonLd.toDateTime(leg[4] + ' ' + leg[5], 'yyyy/MM/dd hh:mm', 'jp');
    res.reservationFor.arrivalAirport.name = leg[6];
    res.reservationFor.arrivalTerminal = leg[7];
    res.reservationFor.arrivalTime = JsonLd.toDateTime(leg[8] + ' ' + leg[9], 'yyyy/MM/dd hh:mm', 'jp');

    // TODO multiple passengers?
    const pas = content.match(/Name: (.*)\nTitle: .*\n\S{2}\d{1,4}\n├ Advance Seat selection: (.*)\n/);
    res.underName.name = pas[1];
    res.airplaneSeat = pas[2];

    return res;
}
