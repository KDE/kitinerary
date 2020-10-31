/*
   SPDX-FileCopyrightText: 2017-2019 Volker Krause <vkrause@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

function parseSncfPdfText(text) {
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

function parseInouiPdfText(page)
{
    var reservations = new Array();
    var text = page.textInRect(0.0, 0.0, 0.5, 1.0);

    var date = text.match(/(\d+ [^ ]+ \d{4})\n/)
    if (!date)
        return reservations;
    var pos = date.index + date[0].length;
    while (true) {
        var dep = text.substr(pos).match(/(\d+h\d+) +(.*)\n/);
        if (!dep)
            break;
        pos += dep.index + dep[0].length;

        var res = JsonLd.newTrainReservation();
        res.reservationFor.departureTime = JsonLd.toDateTime(date[1] + dep[1], "d MMMM yyyyhh'h'mm", "fr");
        res.reservationFor.departureStation.name = dep[2];

        var arr = text.substr(pos).match(/(\d+h\d+) +(.*)\n/);
        if (!arr)
            break;
        var endPos = arr.index + arr[0].length;
        res.reservationFor.arrivalTime = JsonLd.toDateTime(date[1] + arr[1], "d MMMM yyyyhh'h'mm", "fr");
        res.reservationFor.arrivalStation.name = arr[2];

        var detailsText = text.substr(pos, endPos - arr[0].length);
        var train = detailsText.match(/^ *(.*?) *-/);
        res.reservationFor.trainNumber = train[1];
        var seat = detailsText.match(/Voiture *(\d+) *Place *(\d+)/);
        if (seat) {
            res.reservedTicket.ticketedSeat.seatSection = seat[1];
            res.reservedTicket.ticketedSeat.seatNumber = seat[2];
        }

        reservations.push(res);
        if (endPos == 0)
            break;
        pos += endPos;
    }

    return reservations;
}

// see https://community.kde.org/KDE_PIM/KItinerary/SNCF_Barcodes
function parseSncfBarcode(barcode)
{
    var reservations = new Array();

    var res1 = JsonLd.newTrainReservation();
    res1.reservationNumber = barcode.substr(4, 6);
    res1.underName.familyName = barcode.substr(72, 19);
    res1.underName.givenName = barcode.substr(91, 19);
    res1.reservationFor.departureStation.name = barcode.substr(33, 5);
    res1.reservationFor.departureStation.identifier = "sncf:" + barcode.substr(33, 5);
    res1.reservationFor.arrivalStation.name = barcode.substr(38, 5);
    res1.reservationFor.arrivalStation.identifier = "sncf:" + barcode.substr(38, 5);
    res1.reservationFor.departureDay = JsonLd.toDateTime(barcode.substr(48, 5), "dd/MM", "fr");
    res1.reservationFor.trainNumber = barcode.substr(43, 5);
    res1.reservedTicket.ticketToken = "aztecCode:" + barcode;
    res1.reservedTicket.ticketedSeat.seatingType = barcode.substr(110, 1);
    reservations.push(res1);

    if (barcode.substr(115, 1) != '0') {
        var res2 = JsonLd.clone(res1);
        res2.reservationFor.departureStation.name = barcode.substr(116, 5);
        res2.reservationFor.departureStation.identifier = "sncf:" + barcode.substr(116, 5);
        res2.reservationFor.arrivalStation.name = barcode.substr(121, 5);
        res2.reservationFor.arrivalStation.identifier = "sncf:" + barcode.substr(121, 5);
        res2.reservationFor.trainNumber = barcode.substr(126, 5);
        res2.reservedTicket.ticketedSeat.seatingType = barcode.substr(115, 1);
        reservations.push(res2);
    }

    return reservations;
}

function parsePdf(pdf) {
    var reservations = new Array();

    var barcode = null;
    for (var i = 0; i < pdf.pageCount; ++i) {
        var page = pdf.pages[i];
        var nextBarcode = null;
        var images = page.images;
        for (var j = 0; j < images.length && !nextBarcode; ++j) {
            nextBarcode = Barcode.decodeAztec(images[j]);
            if (nextBarcode.substr(0, 4) !== "i0CV")
                nextBarcode = null;
        }
        // Guard against tickets with 3 or more legs, with the second page for the 3rd and subsequent
        // leg repeating the barcode of the first two legs. One would expect the barcode for the following
        // legs there, but that doesn't even seem to exists in the sample documents I have for this...
        barcode = (nextBarcode && nextBarcode != barcode) ? nextBarcode : null;
        if (barcode) {
            var barcodeRes = barcode ? parseSncfBarcode(barcode) : null;
        }

        var legs = parseSncfPdfText(page.text);
        if (legs.length == 0) {
            legs = parseInouiPdfText(page);
        }
        if (legs.length > 0) {
            for (var j = 0; j < legs.length; ++j) {
                if (barcode) {
                    legs[j].underName = barcodeRes[j].underName;
                    legs[j].reservedTicket.ticketToken = "aztecCode:" + barcode;
                    legs[j].reservationFor.departureStation.identifier = barcodeRes[j].reservationFor.departureStation.identifier;
                    legs[j].reservationFor.arrivalStation.identifier = barcodeRes[j].reservationFor.arrivalStation.identifier;
                    legs[j].reservedTicket.ticketedSeat.seatingType = barcodeRes[j].reservedTicket.ticketedSeat.seatingType;
                }
                reservations.push(legs[j]);
            }
        } else {
            reservations = reservations.concat(barcodeRes);
        }
    }

    return reservations;
}

function parseSecutixPdf(pdf)
{
    // see https://community.kde.org/KDE_PIM/KItinerary/SNCF_Barcodes#SNCF_Secutix_Tickets
    var res = JsonLd.newTrainReservation();
    var code = Barcode.byteArrayToString(Context.barcode);
    res.reservationNumber = code.substr(268, 9);
    res.reservationFor.departureStation.name = code.substr(277, 5);
    res.reservationFor.departureStation.identifier = "sncf:" + code.substr(277, 5);
    res.reservationFor.arrivalStation.name = code.substr(282, 5);
    res.reservationFor.arrivalStation.identifier = "sncf:" + code.substr(282, 5);
    res.reservationFor.departureDay = JsonLd.toDateTime(code.substr(343, 8), "ddMMyyyy", "fr");
    res.reservedTicket.ticketedSeat.seatingType = code.substr(351, 1);
    res.reservedTicket.ticketToken = "aztecbin:" + Barcode.toBase64(Context.barcode);
    res.underName.familyName = code.substr(376, 19);
    res.underName.givenName = code.substr(395, 19);

    var text = pdf.pages[Context.pdfPageNumber].text;
    var pnr = text.match(res.reservationNumber + '[^\n]* ([A-Z0-9]{6})\n');
    res.reservationNumber = pnr[1];
    var dep = text.match(/Départ [^ ]+ (\d+\.\d+\.\d+) à (\d+:\d+) [^ ]+ (.*)\n/);
    var arr = text.match(/Arrivée [^ ]+ (\d+\.\d+\.\d+) à (\d+:\d+) [^ ]+ (.*)\n\s+(.*)\n/);
    res.reservationFor.departureStation.name = dep[3];
    res.reservationFor.departureTime = JsonLd.toDateTime(dep[1] + dep[2], "dd.MM.yyyyhh:mm", "fr");
    res.reservationFor.arrivalStation.name = arr[3];
    res.reservationFor.arrivalTime = JsonLd.toDateTime(arr[1] + arr[2], "dd.MM.yyyyhh:mm", "fr");
    res.reservationFor.trainNumber = arr[4];

    return res;
}

function parseOuigoEmail(html)
{
    if (html.eval('//*[@data-select="travel-summary-reference"]').length > 0) {
        return parseOuigoSummary(html);
    } else {
        return parseOuigoConfirmation(html);
    }
}

function parseOuigoSummary(html)
{
    // TODO extract passenger names
    var res = JsonLd.newTrainReservation();
    res.reservationFor.departureStation.name = html.eval('//*[@data-select="travel-summary-origin"]')[0].content;
    res.reservationFor.arrivalStation.name = html.eval('//*[@data-select="travel-summary-destination"]')[0].content;
    res.reservationNumber = html.eval('//*[@data-select="travel-summary-reference"]')[0].content;

    var depTimeStr = html.eval('//*[@data-select="travel-departureDate"]')[0].recursiveContent;
    var depTime = depTimeStr.match(/(\d+ [^ ]+ \d+) +[^ ]+ (\d+:\d+)/);
    if (depTime) {
        res.reservationFor.departureTime = JsonLd.toDateTime(depTime[1] + depTime[2], "dd MMMM yyyyhh:mm", "fr");
    } else {
        depTime = depTimeStr.match(/(\d+ [^ ]+) +[^ ]+ +(\d+[:h]\d+)/);
        res.reservationFor.departureTime = JsonLd.toDateTime(depTime[1] + depTime[2].replace('h', ':'), "dd MMMMhh:mm", "fr");
    }

    var trainNum = html.eval('//*[@class="passenger-detail__equipment"]');
    res.reservationFor.trainNumber = trainNum[0].content + " " + trainNum[1].content;
    return res;
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
        res.reservedTicket.ticketToken = "azteccode:" + Context.barcode;
    }
    return res;
}
