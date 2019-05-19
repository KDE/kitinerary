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
    var bookingRef = text.match(/DOSSIER VOYAGE : +([A-Z0-9]{6})/);

    var pos = 0;
    while (true) {
        var header = text.substr(pos).match(/ +Départ \/ Arrivée.*\n/);
        if (!header)
            break;
        var index = header.index + header[0].length;

        var res = JsonLd.newObject("TrainReservation");
        res.reservedTicket = JsonLd.newObject("Ticket");
        res.reservationNumber = bookingRef[1];
        res.reservationFor = JsonLd.newObject("TrainTrip");

        var depLine = text.substr(pos + index).match(/\n {2,3}([\w -]+?)  +(\d{2}\/\d{2}) à (\d{2}h\d{2})/);
        if (!depLine)
            break;
        index += depLine.index + depLine[0].length;
        res.reservationFor.departureStation = JsonLd.newObject("TrainStation");
        res.reservationFor.departureStation.name = depLine[1];
        res.reservationFor.departureTime = JsonLd.toDateTime(depLine[2] + " " + depLine[3], "dd/MM hh'h'mm", "fr");

        var arrLine = text.substr(pos + index).match(/\n {2,3}([\w -]+?)  +(\d{2}\/\d{2}) à (\d{2}h\d{2})/);
        if (!arrLine)
            break;
        index += arrLine.index + arrLine[0].length;
        res.reservationFor.arrivalStation = JsonLd.newObject("TrainStation");
        res.reservationFor.arrivalStation.name = arrLine[1];
        res.reservationFor.arrivalTime = JsonLd.toDateTime(arrLine[2] + " " + arrLine[3], "dd/MM hh'h'mm", "fr");

        // parse seat, train number, etc from the text for one leg
        // since the stations are vertically centered, the stuff we are looking for might be at different
        // positions relative to them
        var legText = text.substring(pos + header.index + header[0].length, pos + index);
        var trainNumber = legText.match(/TRAIN N° ?(\d{3,4})/);
        if (trainNumber)
            res.reservationFor.trainNumber = trainNumber[1];
        var seatRes = legText.match(/(VOITURE|COACH) (\d+) - PLACE (\d+)/);
        if (seatRes) {
            res.reservedTicket.ticketedSeat = JsonLd.newObject("Seat");
            res.reservedTicket.ticketedSeat.seatSection = seatRes[2];
            res.reservedTicket.ticketedSeat.seatNumber = seatRes[3];
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

    for (var i = 0; i < pdf.pageCount; ++i) {
        var page = pdf.pages[i];

        // barcode format:
        // 'i0CV'
        // 6x PNR
        // 9x document id
        // '1211'
        // dd/MM/yyyy birthdate
        // 2x 5x gare & connextion ids of the first leg
        // 5x train number first leg
        // dd/MM travel date
        // 18x client id
        // 19x family name
        // 19x given name
        // 1x class
        // 4x stuff
        // '1' to indicate a second leg, '0' otherwise
        // 2x 5x gare & connexion ids for the second leg
        // 5x train number second leg
        var barcode = null;
        var images = page.imagesInRect(0.75, 0, 1, 0.75);
        for (var j = 0; j < images.length && !barcode; ++j) {
            barcode = Barcode.decodeAztec(images[j]);
            if (barcode.substr(0, 4).toUpperCase() !== "I0CV")
                barcode = null;
        }

        var underName = null;
        if (barcode) {
            var underName = JsonLd.newObject("Person");
            underName.familyName = barcode.substring(73, 91).trim();
            underName.givenName = barcode.substring(92, 110).trim();
        }

        var legs = parseText(page.text);
        for (var j = 0; j < legs.length; ++j) {
            if (barcode) {
                legs[j].underName = underName;
                legs[j].reservedTicket.ticketToken = "aztecCode:" + barcode;
                legs[j].reservationFor.departureStation.identifier = "sncf:" + barcode.substr(j == 0 ? 33 : 116, 5);
                legs[j].reservationFor.arrivalStation.identifier = "sncf:" + barcode.substr(j == 0 ? 38 : 121, 5);
            }
            reservations.push(legs[j]);
        }
    }

    return reservations;
}

function parseHtmlConfirmation(html)
{
    var reservations = new Array();

    var pnr = html.eval("//*[@class=\"pnr-ref\"]/*[@class=\"pnr-info\"]");

    var productDts = html.eval("//*[@class=\"product-travel-date\"]");
    var productDetails = html.eval("//table[@class=\"product-details\"]");
    for (productDetailIdx in productDetails) {
        // date is in the table before us
        var dt = productDts[productDetailIdx].content.replace(/\S+ (.*)/, "$1");

        var segmentDetail = productDetails[productDetailIdx].eval(".//td")[0];
        var res = null;
        while (segmentDetail && !segmentDetail.isNull) {
            var cls = segmentDetail.attribute("class");
            if (cls.includes("segment-departure")) {
                res = JsonLd.newObject("TrainReservation");
                res.reservationFor = JsonLd.newObject("TrainTrip");
                res.reservationFor.departureStation = JsonLd.newObject("TrainStation");
                res.reservationFor.arrivalStation = JsonLd.newObject("TrainStation");
                if (pnr.length > 0) {
                    res.reservationNumber = pnr[0].content;
                }

                res.reservationFor.departureTime = JsonLd.toDateTime(dt + segmentDetail.content, "dd MMMMhh'h'mm", "fr");
                segmentDetail = segmentDetail.nextSibling;
                res.reservationFor.departureStation.name = segmentDetail.content;
            }
            else if (cls.includes("segment-arrival")) {
                res.reservationFor.arrivalTime = JsonLd.toDateTime(dt + segmentDetail.content, "dd MMMMhh'h'mm", "fr");
                segmentDetail = segmentDetail.nextSibling;
                res.reservationFor.arrivalStation.name = segmentDetail.content;
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
