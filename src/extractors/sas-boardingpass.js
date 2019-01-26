/*
   Copyright (c) 2018 Sune Vuorela <sune@kde.org>

   This library is free software; you can redistribute it and/or modify it
   under the terms of the GNU Library General Public License as published by
   the Free Software Foundation; either version 2 of the License, or (at your
   option) any later version.

   This library is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
   FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
   License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to the
   Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
   02110-1301, USA.
*/

/*
There seems to just be a pdf with at least 3 different formats in my collection.
The newest one seems to have a first line that starts with BOARDING PASS, and the header of the actual data is
FLIGHT                          DATE       TIME            FROM              TO           CLASS             GATE       SEQ# ZONE    SEAT      BOARDING

The older ones starts with a different first line, and have different headers.

I wonder if that's something that could be used for detecting types. But the older formats in my collection are from first half of 2017 and older. 
But we need more data.

 */

function extractInformation(page) {
    var text = page.textInRect(0, 0, 1, 1);
    if (!text.match(/BOARDING PASS/))
        return null;

    var res = JsonLd.newObject("FlightReservation");
    res.reservationFor = JsonLd.newObject("Flight");
    res.reservedTicket = JsonLd.newObject("Ticket");

    var images = page.imagesInRect(0.5, 0, 1, 0.5);
    for (var i = 0; i < images.length; ++i) {
        if (images[i].height < 300 && images[i].width < images[i].height)
        {
            var barcode = Barcode.decodePdf417(images[i]);
            if (barcode !== "")
            {
                res.reservedTicket.ticketToken = "aztecCode:" + Barcode.decodePdf417(images[i]);
                break;
            }
        }
    }

    var lines = text.split('\n');

    var state = 0;
    var lastFlight = null;
    var flights = [];
    for(var i = 0 ; i < lines.length; i++) {
        var line = lines[i];
        if (line.match(/FLIGHT\s+DATE\s+TIME\s+FROM\s+TO\s+CLASS\s+GATE\s+SEQ#\s+ZONE\s+SEAT\s+BOARDING/i))
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
                state = 0;
                continue;
            }
            lastFlight.daymonth = times[1];
            lastFlight.departureTime = times[2];
            lastFlight.boardingTime = times[3];
            state = 2;
            continue;
        }
        if (state === 2) {
            var year = line.match(/\d{4}/);
            lastFlight.year = year;
            state = 0;
            flights.push(lastFlight);
            continue;
        }
    }
    if (lastFlight) {
        res.reservationFor.departureTime = JsonLd.toDateTime(lastFlight.daymonth + " " + lastFlight.year + " " + lastFlight.departureTime, "dd MMM yyyy hh:mm", "en");
        res.reservationFor.boardingTime  = JsonLd.toDateTime(lastFlight.daymonth + " " + lastFlight.year + " " + lastFlight.boardingTime , "dd MMM yyyy hh:mm", "en");
    } else {
        return null;
    }

    return res;
}

function main(pdf) {
    var result = new Array();

    for (var i = 0; i < pdf.pageCount; ++i) {
        var page = pdf.pages[i];
        result.push(extractInformation(page));
    }

    return result;
}
