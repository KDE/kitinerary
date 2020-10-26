/*
   Copyright (c) 2017-2019 Volker Krause <vkrause@kde.org>

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

function parseText(text) {
    var reservations = new Array();
    var bookingRef = text.match(/(?:DOSSIER VOYAGE|BOOKING FILE REFERENCE) : +([A-Z0-9]{6})/);

    var pos = 0;
    while (true) {
        var header = text.substr(pos).match(/ +(?:Départ \/ Arrivée|Departure \/ Arrival).*\n/);
        if (!header)
            break;
        var index = header.index + header[0].length;

        var res = JsonLd.newTrainReservation();
        res.reservationNumber = bookingRef[1];

        var depLine = text.substr(pos + index).match(/\n {2,3}([\w -]+?)  +(\d{2}\/\d{2}) (?:à|at) (\d{2}h\d{2})/);
        if (!depLine)
            break;
        index += depLine.index + depLine[0].length;
        res.reservationFor.departureStation.name = depLine[1];
        res.reservationFor.departureTime = JsonLd.toDateTime(depLine[2] + " " + depLine[3], "dd/MM hh'h'mm", "fr");

        var arrLine = text.substr(pos + index).match(/\n {2,3}([\w -]+?)  +(\d{2}\/\d{2}) (?:à|at) (\d{2}h\d{2})/);
        if (!arrLine)
            break;
        index += arrLine.index + arrLine[0].length;
        res.reservationFor.arrivalStation.name = arrLine[1];
        res.reservationFor.arrivalTime = JsonLd.toDateTime(arrLine[2] + " " + arrLine[3], "dd/MM hh'h'mm", "fr");

        // parse seat, train number, etc from the text for one leg
        // since the stations are vertically centered, the stuff we are looking for might be at different
        // positions relative to them
        var legText = text.substring(pos + header.index + header[0].length, pos + index);
        var trainNumber = legText.match(/TRAIN (?:N°|NUMBER) ?(\d{3,5})/);
        if (trainNumber)
            res.reservationFor.trainNumber = trainNumber[1];
        var seatRes = legText.match(/(?:VOITURE|COACH) (\d+) - PLACE (\d+)/);
        if (seatRes) {
            res.reservedTicket.ticketedSeat.seatSection = seatRes[1];
            res.reservedTicket.ticketedSeat.seatNumber = seatRes[2];
        }

        reservations.push(res);
        if (index == 0)
            break;
        pos += index;
    }

    return reservations;
}

function parsePdf(pdf) {
    var reservations = new Array();

    var barcode = null;
    for (var i = 0; i < pdf.pageCount; ++i) {
        var page = pdf.pages[i];

        // barcode format: (see https://community.kde.org/KDE_PIM/KItinerary/SNCF_Barcodes)
        // 'i0CV'
        // 6x PNR
        // 9x document id / e-ticket number
        // '1211'
        // dd/MM/yyyy birthdate
        // 2x 5x SNCF station code of the first leg
        // 5x train number first leg
        // dd/MM travel date
        // 18x client id
        // 19x family name
        // 19x given name
        // 1x class first leg ('1' or '2')
        // 4x tariff/price code
        // 1x class second leg ('1' or '2'; '0' if there is no second leg)
        // 2x 5x SNCF station code for the second leg
        // 5x train number second leg
        var nextBarcode = null;
        var images = page.imagesInRect(0.75, 0, 1, 0.75);
        for (var j = 0; j < images.length && !nextBarcode; ++j) {
            nextBarcode = Barcode.decodeAztec(images[j]);
            if (nextBarcode.substr(0, 4).toUpperCase() !== "I0CV")
                nextBarcode = null;
        }
        // Guard against tickets with 3 or more legs, with the second page for the 3rd and subsequent
        // leg repeating the barcode of the first two legs. One would expect the barcode for the following
        // legs there, but that doesn't even seem to exists in the sample documents I have for this...
        barcode = (nextBarcode && nextBarcode != barcode) ? nextBarcode : null;

        var underName = null;
        if (barcode) {
            var underName = JsonLd.newObject("Person");
            underName.familyName = barcode.substr(72, 19).trim();
            underName.givenName = barcode.substr(91, 19).trim();
        }

        var legs = parseText(page.text);
        for (var j = 0; j < legs.length; ++j) {
            if (barcode) {
                legs[j].underName = underName;
                legs[j].reservedTicket.ticketToken = "aztecCode:" + barcode;
                legs[j].reservationFor.departureStation.identifier = "sncf:" + barcode.substr(j == 0 ? 33 : 116, 5);
                legs[j].reservationFor.arrivalStation.identifier = "sncf:" + barcode.substr(j == 0 ? 38 : 121, 5);
                legs[j].reservedTicket.ticketedSeat.seatingType = barcode.substr(j == 0 ? 110 : 115, 1);
            }
            reservations.push(legs[j]);
        }
    }

    return reservations;
}

function parseOuigoConfirmation(html)
{
    var reservations = new Array();

    var pnr = html.eval('//*[@class="pnr-ref"]/*[@class="pnr-info"]');
    var pnrOuigo = html.eval('//*[@class="pnr-info-digital pnr-info-digital-ouigo"]');
    var passengerName = html.eval('//*[@class="passenger"]/*[@class="name"]');

    var productDts = html.eval('//*[@class="product-travel-date"]');
    var productDetails = html.eval('//table[@class="product-details"]');
    var passengerDetails = html.eval('//table[@class="passengers"]');
    for (productDetailIdx in productDetails) {
        // date is in the table before us
        var dt = productDts[productDetailIdx].content.replace(/\S+ (.*)/, "$1");

        var segmentDetail = productDetails[productDetailIdx].eval(".//td")[0];
        var placement = passengerDetails[productDetailIdx].eval('.//td[@class="placement "]'); // yes, there is a space behind placement there...
        var seat = placement[0].content.match(/Voiture (.*?) - Place (.*?) /);
        var res = null;
        while (segmentDetail && !segmentDetail.isNull) {
            var cls = segmentDetail.attribute("class");
            if (cls.includes("segment-departure")) {
                res = JsonLd.newTrainReservation();
                res.reservationFor.departureTime = JsonLd.toDateTime(dt + segmentDetail.content, "d MMMMhh'h'mm", "fr");
                segmentDetail = segmentDetail.nextSibling;
                res.reservationFor.departureStation.name = segmentDetail.content;
                if (seat) {
                    res.reservedTicket.ticketedSeat.seatSection = seat[1];
                    res.reservedTicket.ticketedSeat.seatNumber = seat[2];
                }
            }
            else if (cls.includes("segment-arrival")) {
                res.reservationFor.arrivalTime = JsonLd.toDateTime(dt + segmentDetail.content, "d MMMMhh'h'mm", "fr");
                segmentDetail = segmentDetail.nextSibling;
                res.reservationFor.arrivalStation.name = segmentDetail.content;

                if (res.reservationFor.trainName == "OUIGO" && pnrOuigo.length) {
                    res.reservationNumber = pnrOuigo[0].content;
                } else if (pnr.length) {
                    res.reservationNumber = pnr[0].content;
                }
                if (passengerName.length) {
                    res.underName.name = passengerName[0].content;
                }

                // HACK drop invalid elements so the structured fallback kicks in correctly
                // this should be done automatically in the engine
                if (res.reservationFor.departureTime > 0)
                    reservations.push(res);
            }
            else if (cls === "segment") {
                res.reservationFor.trainName = segmentDetail.content;
            }
            else if (cls === "segment-ref-train") {
                res.reservationFor.trainNumber = segmentDetail.content;
            }

            segmentDetail = segmentDetail.nextSibling.isNull ? segmentDetail.parent.nextSibling.firstChild : segmentDetail.nextSibling;
        }
    }

    return reservations;
}

function parseOuigoTicket(pdf) {
    var text = pdf.pages[0].textInRect(0, 0, 0.5, 1);

    var res = JsonLd.newTrainReservation();
    res.reservationNumber = text.match(/numéro de réservation est\s*:\s*([\w]{6})\n/)[1];
    var trip = text.match(/(\d{2} .+ \d{4})\n\s*(\d{2}h\d{2})\s*(.*?)\n\s*(\d{2}h\d{2})\s*(.*?)\n/);
    res.reservationFor.departureStation.name = trip[3];
    res.reservationFor.departureTime = JsonLd.toDateTime(trip[1] + trip[2], "dd MMMM yyyyhh'h'mm", "fr");
    res.reservationFor.arrivalStation.name = trip[5];
    res.reservationFor.arrivalTime = JsonLd.toDateTime(trip[1] + trip[4], "dd MMMM yyyyhh'h'mm", "fr");

    res.reservationFor.trainNumber = text.match(/N°\s*(\S+)/)[1];

    var seat = text.match(/Voiture\s*(\S+)\s*Place\s*(\S+)/);
    res.reservedTicket.ticketedSeat.seatSection = seat[1];
    res.reservedTicket.ticketedSeat.seatNumber = seat[2];
    if (Context.barcode) {
        res.reservedTicket.ticketToken = "acteccode:" + Context.barcode;
    }
    return res;
}
