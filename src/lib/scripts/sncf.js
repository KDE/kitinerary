/*
   SPDX-FileCopyrightText: 2017-2022 Volker Krause <vkrause@kde.org>
   SPDX-License-Identifier: LGPL-2.0-or-later
*/

// see https://community.kde.org/KDE_PIM/KItinerary/SNCF_Barcodes#Tariff_Codes
const tariffs = {
    'CF00': 'Ayant Droit Avec Fichet',
    'CF90': 'Ayant Droit Sans Fichet',
    'CJ11': 'Carte Jeune',
    'CW00': 'Carte Advantage Adulte',
    'CW11': 'Carte Advantage Adulte',
    'CW12': 'Carte Advantage Adulte',
    'CW25': 'Carte Advantage Adulte',
    'EF11': 'Carte Enfant+',
    'EF99': 'Carte Enfant+',
    'FZ71': 'Max Actif',
    'IR00': 'Interrail',
    'IR01': 'Interrail',
    'JE00': 'Carte Jeune',
    'LB00': 'Carte Liberté',
    'SE11': 'Carte Advantage Senior',
    'SR50': 'Carte Senior'
};

function parseSncfPdfText(text) {
    var reservations = new Array();
    const bookingRef = text.match(/(?:DOSSIER VOYAGE|BOOKING FILE REFERENCE|REFERENCE NUMBER|REISEREFERENZ) ?: +([A-Z0-9]{6})/);
    const price = text.match(/(\d+,\d\d EUR)/);

    var pos = 0;
    while (true) {
        var header = text.substr(pos).match(/ +(?:Départ \/ Arrivée|Departure \/ Arrival|Abfahrt \/ Ankunft).*\n/);
        if (!header)
            break;
        var index = header.index + header[0].length;

        var res = JsonLd.newTrainReservation();
        res.reservationNumber = bookingRef[1];

        var depLine = text.substr(pos + index).match(/\n {2,3}([\w -]+?)  +(\d{2}[\/\.]\d{2}) (?:à|at|um) (\d{2}[h:]\d{2})/);
        if (!depLine)
            break;
        index += depLine.index + depLine[0].length;
        res.reservationFor.departureStation.name = depLine[1];
        res.reservationFor.departureTime = JsonLd.toDateTime(depLine[2] + " " + depLine[3], ["dd/MM hh'h'mm", "dd/MM hh:mm", "dd.MM hh:mm"], "fr");

        var arrLine = text.substr(pos + index).match(/\n {2,3}([\w -]+?)  +(\d{2}[\/\.]\d{2}) (?:à|at|um) (\d{2}[h:]\d{2})/);
        if (!arrLine)
            break;
        index += arrLine.index + arrLine[0].length;
        res.reservationFor.arrivalStation.name = arrLine[1];
        res.reservationFor.arrivalTime = JsonLd.toDateTime(arrLine[2] + " " + arrLine[3], ["dd/MM hh'h'mm", "dd/MM hh:mm", "dd.MM hh:mm"], "fr");

        // parse seat, train number, etc from the text for one leg
        // since the stations are vertically centered, the stuff we are looking for might be at different
        // positions relative to them
        var legText = text.substring(pos + header.index + header[0].length, pos + index);
        var trainNumber = legText.match(/(?:TRAIN N°|TRAIN NUMBER|ZUGNUMMER) ?(\d{3,5})/);
        if (trainNumber)
            res.reservationFor.trainNumber = trainNumber[1];
        var seatRes = legText.match(/(?:VOITURE|COACH|WAGEN) (\d+) - (?:PLACE|SEAT|PLATZ) (\d+)/);
        if (seatRes) {
            res.reservedTicket.ticketedSeat.seatSection = seatRes[1];
            res.reservedTicket.ticketedSeat.seatNumber = seatRes[2];
        }

        if (price)
            ExtractorEngine.extractPrice(price[1], res);

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
    const price = page.text.match(/(\d+,\d\d EUR)/);
    var text = page.textInRect(0.0, 0.0, 0.5, 1.0);

    var date = text.match(/(\d+\.? [^ ]+ \d{4})\n/)
    if (!date)
        return reservations;
    var pos = date.index + date[0].length;
    while (true) {
        var dep = text.substr(pos).match(/(\d{2}[h:]\d{2}) +(.*)\n/);
        if (!dep)
            break;
        pos += dep.index + dep[0].length;

        var res = JsonLd.newTrainReservation();
        res.reservationFor.departureTime = JsonLd.toDateTime(date[1] + dep[1], ["d MMMM yyyyhh'h'mm", "dd MMMM yyyyhh:mm", "dd. MMMM yyyyhh:mm"], ["fr", "en", "de", "nl", "it"]);
        res.reservationFor.departureStation.name = dep[2];

        var arr = text.substr(pos).match(/(\d{2}[h:]\d{2}) +(.*)\n/);
        if (!arr)
            break;
        var endPos = arr.index + arr[0].length;
        res.reservationFor.arrivalTime = JsonLd.toDateTime(date[1] + arr[1], ["d MMMM yyyyhh'h'mm", "dd MMMM yyyyhh:mm", "dd. MMMM yyyyhh:mm"], ["fr", "en", "de", "nl", "it"]);
        res.reservationFor.arrivalStation.name = arr[2];

        const detailsText = text.substr(pos, endPos - arr[0].length);
        const train = detailsText.match(/^ *(.*?) *-/);
        res.reservationFor.trainNumber = train[1];
        const aboveStrings = ['Haut', 'Oben', 'Upper deck', 'Alto', 'Hoog'];
        const belowStrings = ['Bas', 'Unten', 'Lower deck']; // TODO these are just guest
        const coachStrings = ['Voiture', 'Wagen', 'Coach', 'Coche', 'Carrozza', 'Rijtuig'];
        const seatStrings = ['Place', 'Platz', 'Seat', 'Plaza', 'Posto', 'Plaats'];
        const regexSeat = new RegExp("(?:" + coachStrings.join('|') + ") *(\\d+) *(" + aboveStrings.join('|') + '|' + belowStrings.join('|') + ")? -? *(?:" + seatStrings.join('|') + ") *(\\d+)", 'i');
        const seat = detailsText.match(regexSeat);
        if (seat) {
            res.reservedTicket.ticketedSeat.seatSection = seat[1];
            res.reservedTicket.ticketedSeat.seatNumber = seat[3];
            if (aboveStrings.includes(seat[2])) {
                res.reservedTicket.ticketedSeat.description = "Upper deck";
            } else if (belowStrings.includes(seat[2])) {
                res.reservedTicket.ticketedSeat.description = "Lower deck";
            }
        }

        if (price)
            ExtractorEngine.extractPrice(price[1], res);

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
    res1.reservedTicket.ticketNumber = barcode.substr(10, 9);
    res1.reservedTicket.ticketedSeat.seatingType = barcode.substr(110, 1);
    res1.programMembershipUsed.programName = tariffs[barcode.substr(111, 4)];
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
                if (barcode && j < barcodeRes.length) {
                    legs[j] = JsonLd.apply(barcodeRes[j], legs[j]);
                }
                reservations.push(legs[j]);
            }
        } else {
            reservations = reservations.concat(barcodeRes);
        }
    }

    return reservations;
}

function parseSecutixPdfItineraryV1(text, res)
{
    var reservations = new Array();
    var pos = 0;
    while (true) {
        var dep = text.substr(pos).match(/Départ [^ ]+ (\d+\.\d+\.\d+) à (\d+:\d+) [^ ]+ (.*)\n/);
        if (!dep)
            break;
        pos += dep.index + dep[0].length;
        var arr = text.substr(pos).match(/Arrivée [^ ]+ (\d+\.\d+\.\d+) à (\d+:\d+) [^ ]+ (.*)\n\s+(.*)\n/);
        if (!arr)
            break;
        pos += arr.index + arr[0].length;

        var leg = JsonLd.newTrainReservation();
        leg.reservationFor.departureStation.name = dep[3];
        leg.reservationFor.departureTime = JsonLd.toDateTime(dep[1] + dep[2], "dd.MM.yyyyhh:mm", "fr");
        leg.reservationFor.arrivalStation.name = arr[3];
        leg.reservationFor.arrivalTime = JsonLd.toDateTime(arr[1] + arr[2], "dd.MM.yyyyhh:mm", "fr");
        leg.reservationFor.trainNumber = arr[4];
        leg.underName = res.underName;
        leg.reservationNumber = res.reservationNumber;
        leg.reservedTicket = res.reservedTicket;
        leg.programMembershipUsed = res.programMembershipUsed;

        reservations.push(leg);
    }
    return reservations;
}

function parseSecutixPdfItineraryV2(text, res)
{
    let reservations = [];
    let pos = 0;
    while (true) {
        const trip = text.substr(pos).match(/ *(\d\dh\d\d)\n(.*)\n(.*)\n([\S\s]*?) *(\d\dh\d\d)\n(.*)\n/);
        if (!trip)
            break;
        pos += trip.index + trip[0].length;

        let leg = JsonLd.newTrainReservation();
        leg.reservationFor.departureStation.name = trip[2];
        leg.reservationFor.departureDay = res.reservationFor.departureDay;
        leg.reservationFor.departureTime = JsonLd.toDateTime(trip[1], "hh'h'mm", "fr");
        leg.reservationFor.arrivalStation.name = trip[6];
        leg.reservationFor.arrivalTime = JsonLd.toDateTime(trip[5], "hh'h'mm", "fr");
        leg.reservationFor.trainNumber = trip[3];
        leg.underName = res.underName;
        leg.reservationNumber = res.reservationNumber;
        leg.reservedTicket = res.reservedTicket;
        leg.programMembershipUsed = res.programMembershipUsed;

        const seat = trip[4].match(/Voiture (\d+) - Place (\d+)\n/);
        if (seat) {
            leg.reservedTicket.ticketedSeat.seatSection = seat[1];
            leg.reservedTicket.ticketedSeat.seatNumber = seat[2];
        }

        reservations.push(leg);
    }
    return reservations;
}

function parseSecutix(barcode)
{
    // see https://community.kde.org/KDE_PIM/KItinerary/SNCF_Barcodes#SNCF_Secutix_Tickets
    let res = JsonLd.newTrainReservation();
    const code = ByteArray.decodeLatin1(barcode.slice(260));
    res.reservationFor.provider.identifier = 'uic:' + code.substr(4, 4);
    res.reservationNumber = code.substr(8, 9);
    res.reservationFor.departureStation.name = code.substr(17, 5);
    res.reservationFor.departureStation.identifier = "sncf:" + code.substr(17, 5);
    res.reservationFor.arrivalStation.name = code.substr(22, 5);
    res.reservationFor.arrivalStation.identifier = "sncf:" + code.substr(22, 5);
    res.reservationFor.departureDay = JsonLd.toDateTime(code.substr(83, 8), "ddMMyyyy", "fr");
    res.reservedTicket.ticketedSeat.seatingType = code.substr(91, 1);
    res.reservedTicket.ticketNumber = code.substr(8, 9);
    res.reservedTicket.ticketToken = "aztecbin:" + ByteArray.toBase64(barcode);
    res.underName.familyName = code.substr(116, 19);
    res.underName.givenName = code.substr(135, 19);
    res.programMembershipUsed.programName = tariffs[code.substr(92, 4)];
    res.reservedTicket.totalPrice = code.substr(226, 10) / 100;
    res.reservedTicket.priceCurrency = 'EUR';
    return res;
}

function parseSecutixPdf(pdf, node, triggerNode)
{
    let res = triggerNode.result[0];
    const text = pdf.pages[triggerNode.location].text;
    let pnr = text.match(res.reservationNumber + '[^\n]* ([A-Z0-9]{6})\n');
    let layoutVersion = 1;
    if (!pnr) {
        pnr = text.match(/(?:PAO|REF)\s*:\s*([A-Z0-9]{6,8})\n/);
        layoutVersion = 2;
    }
    res.reservationNumber = pnr[1];

    var itineraryText = pdf.pages[triggerNode.location].textInRect(0.0, 0.0, 0.5, 1.0);
    var reservations = layoutVersion == 1 ? parseSecutixPdfItineraryV1(itineraryText, res) : parseSecutixPdfItineraryV2(itineraryText, res);
    if (reservations.length == 0)
        return res;

    reservations[0].reservationFor.departureStation.identifier = res.reservationFor.departureStation.identifier;
    reservations[reservations.length - 1].reservationFor.arrivalStation.identifier = res.reservationFor.arrivalStation.identifier;
    for (r of reservations) {
        r.reservationFor.provider = res.reservationFor.provider;
    }
    for (i in reservations) {
        if (reservations[i].reservationFor.trainNumber.startsWith("CAR"))
            reservations[i] = JsonLd.trainToBusReservation(reservations[i]);
    }

    return reservations;
}

function parseOuiEmail(html)
{
    if (html.eval('//*[@data-select="travel-summary"]').length > 0) {
        return parseOuiSummary(html);
    } else {
        return parseOuiConfirmation(html);
    }
}

function parseOuiSummaryTime(htmlElem)
{
    var timeStr = htmlElem[0].recursiveContent;
    var time = timeStr.match(/(\d+ [^ ]+ \d+) +[^ ]+ (\d+:\d+)/);
    if (time) {
        return JsonLd.toDateTime(time[1] + time[2], "d MMMM yyyyhh:mm", "fr");
    }
    time = timeStr.match(/(\d+\.? [^ ]+(?: \d{4})?) +[^ ]+ +(\d+[:h]\d+)/);
    return JsonLd.toDateTime(time[1] + ' ' + time[2].replace('h', ':'), ["d MMMM hh:mm", "d. MMMM yyyy hh:mm"], ["fr", "en", "de"]);
}

function parseOuiSummary(html)
{
    var reservations = new Array();

    var travelSummaries = html.eval('//*[@data-select="travel-summary"]');
    var passengersDetails = html.eval('//*[@data-select="passengers-details"]');
    var individualPricesFound = html.eval('//*[@data-select="travel-summary-price"]').length === travelSummaries.length;
    for (travelSummaryIdx in travelSummaries) {
        // TODO extract passenger names
        var travelSummary = travelSummaries[travelSummaryIdx];
        var passengersDetail = passengersDetails[travelSummaryIdx];

        var res = JsonLd.newTrainReservation();
        const origins = travelSummary.eval('.//*[@data-select="travel-summary-origin"]');
        res.reservationFor.departureStation.name = origins[0].content;
        const destinations = travelSummary.eval('.//*[@data-select="travel-summary-destination"]');
        res.reservationFor.arrivalStation.name = destinations[0].content;
        res.reservationNumber = travelSummary.eval('.//*[@data-select="travel-summary-reference"]')[0].content;

        res.reservationFor.departureTime = parseOuiSummaryTime(travelSummary.eval('.//*[@data-select="travel-departureDate"]'));

        var trainNum = passengersDetail.eval('.//*[@data-select="passenger-detail-outwardFares"]//*[@class="passenger-detail__equipment"]');
        if (trainNum.length == 2 || trainNum[1].content == trainNum[3].content) {
            // can occur multiple times for multi-leg journeys or multiple passengers
            // we don't have information about connections on multi-leg journeys, so omit the train number in that case
            res.reservationFor.trainNumber = trainNum[0].content + " " + trainNum[1].content;
        }

        const price = travelSummary.eval('.//*[@data-select="travel-summary-price"]')
        if (individualPricesFound && price.length > 0)
            ExtractorEngine.extractPrice(price[0].recursiveContent, res);

        reservations.push(res);

        // check if this is a return ticket
        var retourTime = travelSummary.eval('.//*[@data-select="travel-returnDate"]');
        if (retourTime.length == 0) {
            continue;
        }
        var retour = JsonLd.newTrainReservation();
        retour.reservationFor.departureStation.name = origins[1] ? origins[1].content : res.reservationFor.arrivalStation.name;
        retour.reservationFor.arrivalStation.name = destinations[1] ? destinations[1].content : res.reservationFor.departureStation.name;
        retour.reservationFor.departureTime = parseOuiSummaryTime(retourTime);
        trainNum = passengersDetail.eval('.//*[@data-select="passenger-detail-inwardFares"]//*[@class="passenger-detail__equipment"]');
        if (trainNum.length == 2 || trainNum[1].content == trainNum[3].content) {
            retour.reservationFor.trainNumber = trainNum[0].content + " " + trainNum[1].content;
        }
        if (individualPricesFound && price.length > 0)
            ExtractorEngine.extractPrice(price[0].recursiveContent, retour);

        reservations.push(retour);
    }

    if (!individualPricesFound) {
        const price = html.eval('//*[@class="transaction__total-amount-value"]');
        if (price.length > 0)
            ExtractorEngine.extractPrice(price[0].recursiveContent, reservations);
    }

    return reservations;
}

function parseOuiConfirmation(html)
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

function parseOuigoTicket(pdf, node) {
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

    var barcodes = node.findChildNodes({ scope: "Descendants", mimeType: "text/plain", match: ".*" });
    for (barcode of barcodes) {
        if (barcode.location != undefined) {
            res.reservedTicket.ticketToken = "azteccode:" + barcodes[0].content;
            break;
        }
    }

    const price = text.match(/ (\d+\.\d\d)€/);
    if (price) {
        res.totalPrice = price[1];
        res.priceCurrency = 'EUR';
    }
    return res;
}

function parseTerConfirmation(html) {
    var reservations = new Array();
    const refNum = html.eval('//td[@id="referenceContainer"]')[0].content;
    const name = html.eval('//td[@id="nomReferenceContainer"]')[0].content;
    const journeys = html.eval('//table[@id ="emailTrajet" or @id="emailTrajetRetour"]');
    for (const journey of journeys) {
        var res = JsonLd.newTrainReservation();
        const dt = journey.eval('.//h2')[0].content.match(/ (\d.*)$/)[1];
        res.reservationFor.departureDay = JsonLd.toDateTime(dt, "dd MMMM yyyy", "fr");
        const ps = journey.eval('.//p');
        res.reservationFor.departureStation.name = ps[0].content;
        res.reservationFor.departureTime = JsonLd.toDateTime(ps[1].content.match(/ (\d.*)/)[1], "hh'h'mm", "fr");
        res.reservationFor.arrivalStation.name = ps[2].content;
        res.reservationFor.arrivalTime = JsonLd.toDateTime(ps[3].content.match(/ (\d.*)/)[1], "hh'h'mm", "fr");
        res.reservationNumber = refNum;
        res.underName.name = name;
        reservations.push(res);
    }
    return reservations;
}

function parseOuigoConfirmation(html) {
    var reservations = new Array();
    const refNum = html.eval('//strong')[1].content;
    const tabs = html.eval('//table//table//table[@class = "rsz_320"]');
    for (const tab of tabs) {
        const text = tab.recursiveContent;
        if (!text.match(/TRAJET/)) {
            continue;
        }

        var idx = 0;
        while (true) {
            const date = text.substr(idx).match(/\w+ (\d{1,2} \w+ \d{4})/);
            if (!date) {
                break;
            }
            const leg = text.substr(idx).match(/(\d{2}h\d{2})\s+(.*?)\n\s*(\d{2}h\d{2})\s*(.*?)\n\s*TRAIN N° *(.*)\n/);
            var res = JsonLd.newTrainReservation();
            res.reservationNumber = refNum;
            res.reservationFor.departureTime = JsonLd.toDateTime(date[1] + leg[1], "d MMMM yyyyhh'h'mm", "fr");
            res.reservationFor.departureStation.name = leg[2];
            res.reservationFor.arrivalStation.name = leg[4];
            res.reservationFor.arrivalTime = JsonLd.toDateTime(date[1] + leg[3],  "d MMMM yyyyhh'h'mm", "fr");
            res.reservationFor.trainNumber = leg[5];
            reservations.push(res);

            idx += leg[0].length + leg.index;
        }
    }
    return reservations;
}

// see https://community.kde.org/KDE_PIM/KItinerary/SNCF_Barcodes#Carte_Advantage
function parseSncfCarte(code) {
    var carte = JsonLd.newObject("ProgramMembership");
    carte.programName = tariffs[code.substr(111, 4)];
    carte.membershipNumber = code.substr(53, 17);
    carte.token = 'azteccode:' + code;
    return carte.programName != undefined ? carte : undefined;
}

function parseSncfCartePdf(pdf, node, barcode) {
    const text = pdf.pages[barcode.location].text;
    var carte = node.result[0];
    carte.member = JsonLd.newObject("Person");
    carte.member.familyName = text.match(/(?:Nom|Name)\s*:\s*(.*)/)[1];
    carte.member.givenName = text.match(/(?:Prénom|Vorname)\s*:\s*(.*)/)[1];
    const validity = text.match(/(?:Du|Vom)\s+(\d{2}[\/\.]\d{2}[\/\.]\d{4}).*(?:au|bis zum)\s+(\d{2}[\/\.]\d{2}[\/\.]\d{4})/);
    carte.validFrom = JsonLd.toDateTime(validity[1], ['dd/MM/yyyy', 'dd.MM.yyyy'], 'fr');
    carte.validUntil = JsonLd.toDateTime(validity[2] + ' 23:59:59', ['dd/MM/yyyy hh:mm:ss', 'dd.MM.yyyy hh:mm:ss'], 'fr');
    return carte;
}

function parseSncfDosipas(dosipas, node) {
    let results = [];
    for (let r of node.result) {
        // fix broken ticket names like "null null nulll null"...
        if (r['@type'] === 'Ticket' && r.name.match(/\bnull\b/))
            r.name = undefined;
        results.push(r)
    }
    return results;
}

// see https://community.kde.org/KDE_PIM/KItinerary/SNCF_Barcodes#SNCF_Normandie_Tickets
// PDF layout matches that of the "secutix" v2
function parseSncfDosipasPdf(pdf, node, triggerNode) {
    let res = {};
    if (triggerNode.result[0]['@type'] == 'TrainReservation') {
        res = triggerNode.result[0];
    } else {
        res = JsonLd.newTrainReservation();
        res.reservedTicket = triggerNode.result[0];
    }

    const page = pdf.pages[triggerNode.location];
    const textRight = page.textInRect(0.5, 0.0, 1.0, 1.0);
    const pnr = textRight.match(/(.*)\n(.*)\n\d{2}\/\d{2}\/\d{4} +(?:PAO|REF)\s*:\s*([A-Z0-9]{6,8})\n/);
    res.reservationNumber = pnr[3];
    res.underName = { '@type': 'Person', givenName: pnr[2], familyName: pnr[1] };

    const textLeft = pdf.pages[triggerNode.location].textInRect(0.0, 0.0, 0.5, 1.0);
    const date = textLeft.match(/(\d{1,2} \S+ \d{4})/)[1];
    res.reservationFor.departureDay = JsonLd.toDateTime(date, 'd MMMM yyyy', 'fr');
    let reservations = parseSecutixPdfItineraryV2(textLeft, res);
    return reservations;
}

function readSSBFreeTextAscii6(ssb, offset, length)
{
    s = "";
    for (let i = 0; i < length; ++i) {
        s += String.fromCharCode(ssb.readNumber(offset + i * 6, 6) + 32);
    }
    return s;
}

function parseSSB(ssb, node)
{
    let res = node.result[0];
    res.reservedTicket.ticketedSeat.seatingType = res.reservedTicket.ticketedSeat.seatingType == 4 ? "1" : "2";
    res.reservedTicket.name = undefined;
    if (ssb.ticketTypeCode === 2) {
        res.reservedTicket.name = tariffs[readSSBFreeTextAscii6(ssb, 239 + 32, 4)];
        res.totalPrice = ssb.readNumber(239 + 148, 16) / 100;
        res.priceCurrency = "EUR";
    }
    return res;
}

function parseEvent(ev)
{
    let res = JsonLd.newTrainReservation();
    const names = ev.description.match(/ +(.*) -> (.*)\n/);
    res.reservationFor.departureStation.name = names[1];
    res.reservationFor.departureTime = JsonLd.readQDateTime(ev, 'dtStart');
    res.reservationFor.arrivalStation.name = names[2];
    res.reservationFor.arrivalTime = JsonLd.readQDateTime(ev, 'dtEnd');
    res.reservationFor.trainNumber = ev.description.match(/ +([A-Z ]+ \d+)\n/i)[1];
    const seat = ev.description.match(/ +(?:VOITURE|COACH|WAGEN) (\d+) - (?:PLACE|SEAT|PLATZ) (\d+)/i);
    if (seat) {
        res.reservedTicket.ticketedSeat.seatSection = seat[1];
        res.reservedTicket.ticketedSeat.seatNumber = seat[2];
    }
    res.reservationNumber = ev.uid.substr(0, 6);
    res.url = ev.url;
    return res;
}
