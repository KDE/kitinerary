/*
   SPDX-FileCopyrightText: 2018 Sune Vuorela <sune@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

function parseDate(day,time, issueDateString) {
   var issueDate = JsonLd.toDateTime(issueDateString, "ddMMMyy","en");
   var ticketDate = JsonLd.toDateTime(day +"" + issueDate.getFullYear() + " " + time, "ddMMMyyyy hh:mm", "en" );
   if (ticketDate < issueDate) {
       ticketDate.setFullYear(issueDate.getFullYear() +1);
   }
   return ticketDate;
}

function extractInformation(page) {
    var text = page.textInRect(0, 0, 1, 1);
    if (text.match(/\s+Electronic Ticket Itinerary and Receipt/)) {
        var res = new Array();
        var lines = text.split(/\n+/);
        var lastFlight = null;
        var bookingRef = text.match(/Booking Reference:\s* ([A-Z0-9]+)\s/);
        var issueDate = text.match(/.*Date of Issue:\s([0-9A-Z]+)/);
        var state = 0;
        var operatedBy = "";
        for(var i = 0 ; i < lines.length; i++ ) {
            if (lines[i].match(/\s+Flight\/Date\s+Route\s+Departure\s+Arrival\s+Latest\s+Flight\s+Baggage/)) {
               state = 1; // headers found
               continue;
            }
            if(lines[i].match(/Ticket Number:.*/)) {
                break;
            }
            if (state === 1) {
               state = 2;
               continue;
            }
            if (state === 2) {
                operatedBy = lines[i].trim();
                state = 3;
                continue;
            }
            if (state ===3) {
                //  Parsing lines like
                //  LH 160 / 27DEC Frankfurt FRA - Leipzig/Halle                             17:00    17:55                 Terminal 1       1PC
                //  SK 1674 / 03JAN Berlin Tegel - Copenhagen Kastrup                        13:30    14:30         12:30                    1PC
                //  SK 675 / 27DEC Copenhagen Kastrup - Frankfurt FRA                        13:40    15:15         12:40 Terminal 3         1PC
                var flightLine = lines[i].match(/([A-Z0-9]{2}) ([0-9]{1,4}) \/ ([0-9]{2}[A-Z]{3})\s+([^0-9]+)([0-9]{2}:[0-9]{2})\s+([0-9]{2}:[0-9]{2})\s+([0-9]{2}:[0-9]{2})?.*/)
                if (flightLine) {
                    var flight = JsonLd.newFlightReservation();
                    flight.reservationNumber = bookingRef[1];
                    flight.reservationFor.flightNumber = flightLine[2];
                    flight.reservationFor.airline.iataCode = flightLine[1];
                    flight.reservationFor.airline.name = operatedBy;
                    flight.reservationFor.departureTime = parseDate(flightLine[3], flightLine[5], issueDate[1] );
                    var airports = flightLine[4].split(" - ");
                    flight.reservationFor.departureAirport.name = airports[0].trim();;
                    flight.reservationFor.arrivalTime = parseDate(flightLine[3], flightLine[6], issueDate[1] );
                    flight.reservationFor.arrivalAirport.name = airports[1].trim();
                    res.push(flight);
                }
                state = 4;
                continue;
            }
            if(state === 4) {
                state = 2;
            }

        }
        return res;

    }
    return null;
}


function main(pdf) {
    var result = new Array();

    for (var i = 0; i < pdf.pageCount; ++i) {
        var page = pdf.pages[i];
        var parseResult = extractInformation(page);
        if (Array.isArray(parseResult)) {
            result = result.concat(parseResult);
        }
    }

    return result;
}
