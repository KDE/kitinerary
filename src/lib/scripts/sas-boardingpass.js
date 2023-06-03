/*
   SPDX-FileCopyrightText: 2018 Sune Vuorela <sune@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

/*
There seems to just be a pdf with at least 3 different formats in my collection.
The newest one seems to have a first line that starts with BOARDING PASS, and the header of the actual data is
FLIGHT                          DATE       TIME            FROM              TO           CLASS             GATE       SEQ# ZONE    SEAT      BOARDING

The older ones starts with a different first line, and have different headers.

I wonder if that's something that could be used for detecting types. But the older formats in my collection are from first half of 2017 and older. 
But we need more data.

 */

function main(pdf, node, triggerNode) {
    const text = pdf.pages[triggerNode.location].text;
    if (!text.match(/BOARDING PASS/))
        return null;

    var lines = text.split('\n');

    var state = 0;
    var flights = [];
    for(var i = 0 ; i < lines.length; i++) {
        var line = lines[i];
        if (line.match(/FLIGHT\s+DATE\s+TIME\s+FROM\s+TO\s+CLASS\s+GATE\s+SEQ#\s+(:?ZONE|GROUP)\s+SEAT\s+BOARDING/i))
        {
            if (state !== 0) console.log("ISSUE")
            lastFlight = {};
            state = 1;
            continue;
        }
        if (state === 0) {
            continue;
        }
        if (state === 1) {
            var times = line.match(/\w+\s+(\d{2} [a-zA-Z]{3})\.?\s+(\d{2}:\d{2})\s+.*(\d{2}:\d{2}).*/);
            if (!times) {
                continue;
            }
            let flight = triggerNode.result[flights.length];
            flight.reservationFor.departureTime = JsonLd.toDateTime(times[1] + ' ' + times[2],'dd MMM hh:mm', 'en');
            flight.reservationFor.boardingTime = JsonLd.toDateTime(times[1] + ' ' + times[3],'dd MMM hh:mm', 'en');
            flights.push(flight);
            continue;
        }
    }

    return flights;
}
