/*
   SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
   SPDX-License-Identifier: LGPL-2.0-or-later
*/

function parseReservation(content) {
    content = content.replace(/\r\n/g, '\n');
    const ref = content.match(/Booking Reference\n(\S{6})/);
    if (!ref)
        return [];

    const passengers = content.split(/\nName: ([^\n]+)\n/);
    const legs = /([A-Z0-9]{2})(\d{1,4})】[^\n]*\n\s*├ (.*)　(.*)\n├ (\d{4}\/\d{2}\/\d{2}).*(\d{2}:\d{2})\n[｜│] ↓\n├ (.*)　(.*)\n└ (\d{4}\/\d{2}\/\d{2}).*(\d{2}:\d{2})/g;
    let reservations = [];
    let leg;
    while ((leg = legs.exec(content)) !== null) {
        let base = JsonLd.newFlightReservation();
        base.reservationNumber = ref[1];
        base.reservationFor.airline.iataCode = leg[1];
        base.reservationFor.flightNumber = leg[2];
        base.reservationFor.departureAirport.name = leg[3];
        base.reservationFor.departureTerminal = leg[4];
        base.reservationFor.departureTime = JsonLd.toDateTime(leg[5] + ' ' + leg[6], 'yyyy/MM/dd hh:mm', 'en');
        base.reservationFor.arrivalAirport.name = leg[7];
        base.reservationFor.arrivalTerminal = leg[8];
        base.reservationFor.arrivalTime = JsonLd.toDateTime(leg[9] + ' ' + leg[10], 'yyyy/MM/dd hh:mm', 'en');

        for (let i = 1; i < passengers.length; i += 2) {
            const flight = passengers[i + 1].match(new RegExp('(?:^|\\n)' + leg[1] + leg[2] + '\\n├ (?:Advance Seat selection|Seat): ([^\\n]+)'));
            if (!flight)
                continue;
            let res = JsonLd.clone(base);
            res.underName.name = passengers[i];
            if (/^\d+[A-Z]$/.test(flight[1]))
                res.airplaneSeat = flight[1];
            reservations.push(res);
        }
    }
    return reservations;
}

function parseHtmlReservation(html, node) {
    let reservations = [];
    const tables = html.eval("//table[tbody/tr/td[@colspan='3']/div]");
    for (const base of node.result) {
        if (base['@type'] !== 'FlightReservation')
            continue;
        let passengersFound = false;
        for (const table of tables) {
            const parents = table.eval('ancestor::table[1]');
            if (parents.length === 0)
                continue;
            const flight = parents[0].recursiveContent.match(/\bMM(\d{1,4})\b/);
            if (!flight || flight[1] !== base.reservationFor.flightNumber)
                continue;
            const rows = table.eval("tbody/tr[td[@colspan='3']/div]");
            for (const row of rows) {
                const cells = row.nextSibling.eval('td');
                if (cells.length !== 3)
                    continue;
                let res = JsonLd.clone(base);
                res.underName = {
                    '@type': 'Person',
                    name: row.recursiveContent.trim().replace(/\s+\([^)]*\)$/, '')
                };
                const seat = cells[0].recursiveContent.trim();
                res.airplaneSeat = /^\d+[A-Z]$/.test(seat) ? seat : undefined;
                reservations.push(res);
                passengersFound = true;
            }
        }
        if (!passengersFound)
            reservations.push(base);
    }
    return reservations;
}
