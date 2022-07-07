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
    'IR00': 'Interrail',
    'IR01': 'Interrail',
    'JE00': 'Carte Jeune',
    'SE11': 'Carte Advantage Senior',
    'SR50': 'Carte Senior'
};

function parseSncfPdfText(text) {
    var reservations = new Array();
    var bookingRef = text.match(/(?:DOSSIER VOYAGE|BOOKING FILE REFERENCE|REFERENCE NUMBER) ?: +([A-Z0-9]{6})/);

    var pos = 0;
    while (true) {
        var header = text.substr(pos).match(/ +(?:Départ \/ Arrivée|Departure \/ Arrival).*\n/);
        if (!header)
            break;
        var index = header.index + header[0].length;

        var res = JsonLd.newTrainReservation();
        res.reservationNumber = bookingRef[1];

        var depLine = text.substr(pos + index).match(/\n {2,3}([\w -]+?)  +(\d{2}\/\d{2}) (?:à|at) (\d{2}[h:]\d{2})/);
        if (!depLine)
            break;
        index += depLine.index + depLine[0].length;
        res.reservationFor.departureStation.name = depLine[1];
        res.reservationFor.departureTime = JsonLd.toDateTime(depLine[2] + " " + depLine[3], ["dd/MM hh'h'mm", "dd/MM hh:mm"], "fr");

        var arrLine = text.substr(pos + index).match(/\n {2,3}([\w -]+?)  +(\d{2}\/\d{2}) (?:à|at) (\d{2}[h:]\d{2})/);
        if (!arrLine)
            break;
        index += arrLine.index + arrLine[0].length;
        res.reservationFor.arrivalStation.name = arrLine[1];
        res.reservationFor.arrivalTime = JsonLd.toDateTime(arrLine[2] + " " + arrLine[3], ["dd/MM hh'h'mm", "dd/MM hh:mm"], "fr");

        // parse seat, train number, etc from the text for one leg
        // since the stations are vertically centered, the stuff we are looking for might be at different
        // positions relative to them
        var legText = text.substring(pos + header.index + header[0].length, pos + index);
        var trainNumber = legText.match(/TRAIN (?:N°|NUMBER) ?(\d{3,5})/);
        if (trainNumber)
            res.reservationFor.trainNumber = trainNumber[1];
        var seatRes = legText.match(/(?:VOITURE|COACH) (\d+) - (?:PLACE|SEAT) (\d+)/);
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
        var dep = text.substr(pos).match(/(\d{2}[h:]\d{2}) +(.*)\n/);
        if (!dep)
            break;
        pos += dep.index + dep[0].length;

        var res = JsonLd.newTrainReservation();
        res.reservationFor.departureTime = JsonLd.toDateTime(date[1] + dep[1], ["d MMMM yyyyhh'h'mm", "dd MMMM yyyyhh:mm"], ["fr", "en"]);
        res.reservationFor.departureStation.name = dep[2];

        var arr = text.substr(pos).match(/(\d{2}[h:]\d{2}) +(.*)\n/);
        if (!arr)
            break;
        var endPos = arr.index + arr[0].length;
        res.reservationFor.arrivalTime = JsonLd.toDateTime(date[1] + arr[1], ["d MMMM yyyyhh'h'mm", "dd MMMM yyyyhh:mm"], ["fr", "en"]);
        res.reservationFor.arrivalStation.name = arr[2];

        var detailsText = text.substr(pos, endPos - arr[0].length);
        var train = detailsText.match(/^ *(.*?) *-/);
        res.reservationFor.trainNumber = train[1];
        var seat = detailsText.match(/(?:Voiture|Coach) *(\d+) *(?:Place|Seat) *(\d+)/);
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
                    legs[j].underName = barcodeRes[j].underName;
                    legs[j].reservedTicket.ticketToken = "aztecCode:" + barcode;
                    legs[j].reservationFor.departureStation.identifier = barcodeRes[j].reservationFor.departureStation.identifier;
                    legs[j].reservationFor.arrivalStation.identifier = barcodeRes[j].reservationFor.arrivalStation.identifier;
                    legs[j].reservedTicket.ticketedSeat.seatingType = barcodeRes[j].reservedTicket.ticketedSeat.seatingType;
                    legs[j].reservationNumber = barcodeRes[j].reservationNumber;
                    legs[j].programMembershipUsed = barcodeRes[j].programMembershipUsed;
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
    var reservations = new Array();
    var pos = 0;
    while (true) {
        var data = text.substr(pos).match(/(\d+h\d+)\n(.*)\n(.*)\n(\d+h\d+)\n(.*)\n/);
        if (!data)
            break;
        pos += data.index + data[0].length;

        var leg = JsonLd.newTrainReservation();
        leg.reservationFor.departureStation.name = data[2];
        leg.reservationFor.departureDay = res.reservationFor.departureDay;
        leg.reservationFor.departureTime = JsonLd.toDateTime(data[1], "hh'h'mm", "fr");
        leg.reservationFor.arrivalStation.name = data[5];
        leg.reservationFor.arrivalTime = JsonLd.toDateTime(data[4], "hh'h'mm", "fr");
        leg.reservationFor.trainNumber = data[3];
        leg.underName = res.underName;
        leg.reservationNumber = res.reservationNumber;
        leg.reservedTicket = res.reservedTicket;
        leg.programMembershipUsed = res.programMembershipUsed;

        reservations.push(leg);
    }
    return reservations;
}

function parseSecutixPdf(pdf, node, triggerNode)
{
    // see https://community.kde.org/KDE_PIM/KItinerary/SNCF_Barcodes#SNCF_Secutix_Tickets
    var res = JsonLd.newTrainReservation();
    var code = ByteArray.decodeLatin1(triggerNode.content.slice(260));
    res.reservationFor.provider.identifier = 'uic:' + code.substr(4, 4);
    res.reservationNumber = code.substr(8, 9);
    res.reservationFor.departureStation.name = code.substr(17, 5);
    res.reservationFor.departureStation.identifier = "sncf:" + code.substr(17, 5);
    res.reservationFor.arrivalStation.name = code.substr(22, 5);
    res.reservationFor.arrivalStation.identifier = "sncf:" + code.substr(22, 5);
    res.reservationFor.departureDay = JsonLd.toDateTime(code.substr(83, 8), "ddMMyyyy", "fr");
    res.reservedTicket.ticketedSeat.seatingType = code.substr(91, 1);
    res.reservedTicket.ticketToken = "aztecbin:" + ByteArray.toBase64(triggerNode.content);
    res.underName.familyName = code.substr(116, 19);
    res.underName.givenName = code.substr(135, 19);
    res.programMembershipUsed.programName = tariffs[code.substr(92, 4)];

    var text = pdf.pages[triggerNode.location].text;
    var pnr = text.match(res.reservationNumber + '[^\n]* ([A-Z0-9]{6})\n');
    var layoutVersion = 1;
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

    return reservations;
}

function parseOuiEmail(html)
{
    if (html.eval('//*[@data-select="travel-summary-reference"]').length > 0) {
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
    time = timeStr.match(/(\d+ [^ ]+) +[^ ]+ +(\d+[:h]\d+)/);
    return JsonLd.toDateTime(time[1] + time[2].replace('h', ':'), "d MMMMhh:mm", "fr");
}

function parseOuiSummary(html)
{
    // TODO extract passenger names
    var res = JsonLd.newTrainReservation();
    const origins = html.eval('//*[@data-select="travel-summary-origin"]');
    res.reservationFor.departureStation.name = origins[0].content;
    const destinations = html.eval('//*[@data-select="travel-summary-destination"]');
    res.reservationFor.arrivalStation.name = destinations[0].content;
    res.reservationNumber = html.eval('//*[@data-select="travel-summary-reference"]')[0].content;

    res.reservationFor.departureTime = parseOuiSummaryTime(html.eval('//*[@data-select="travel-departureDate"]'));

    var trainNum = html.eval('//*[@data-select="passenger-detail-outwardFares"]//*[@class="passenger-detail__equipment"]');
    if (trainNum.length == 2 || trainNum[1].content == trainNum[3].content) {
        // can occur multiple times for multi-leg journeys or multiple passengers
        // we don't have information about connections on multi-leg journeys, so omit the train number in that case
        res.reservationFor.trainNumber = trainNum[0].content + " " + trainNum[1].content;
    }

    // check if this is a return ticket
    var retourTime = html.eval('//*[@data-select="travel-returnDate"]');
    if (retourTime.length == 0) {
        return res;
    }
    var retour = JsonLd.newTrainReservation();
    retour.reservationFor.departureStation.name = origins[1] ? origins[1].content : res.reservationFor.arrivalStation.name;
    retour.reservationFor.arrivalStation.name = destinations[1] ? destinations[1].content : res.reservationFor.departureStation.name;
    retour.reservationFor.departureTime = parseOuiSummaryTime(retourTime);
    trainNum = html.eval('//*[@data-select="passenger-detail-inwardFares"]//*[@class="passenger-detail__equipment"]');
    if (trainNum.length == 2 || trainNum[1].content == trainNum[3].content) {
        retour.reservationFor.trainNumber = trainNum[0].content + " " + trainNum[1].content;
    }

    return [res, retour];
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
            const leg = text.substr(idx).match(/(\d{2}h\d{2})\s+(.*?)\n\s+(\d{2}h\d{2})\s+(.*?)\n\s+TRAIN N° *(.*)\n/);
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
    carte.token = 'aztec:' + code;
    return carte.programName != undefined ? carte : undefined;
}

function parseSncfCartePdf(pdf, node, barcode) {
    const text = pdf.pages[barcode.location].text;
    var carte = node.result[0];
    carte.member = JsonLd.newObject("Person");
    carte.member.familyName = text.match(/Nom\s*:\s*(.*)/)[1];
    carte.member.givenName = text.match(/Prénom\s*:\s*(.*)/)[1];
    const validity = text.match(/Du\s+(\d{2}\/\d{2}\/\d{4})\s+au\s+(\d{2}\/\d{2}\/\d{4})/);
    carte.validFrom = JsonLd.toDateTime(validity[1], 'dd/MM/yyyy', 'fr');
    carte.validTo = JsonLd.toDateTime(validity[2] + ' 23:59:59', 'dd/MM/yyyy hh:mm:ss', 'fr');
    return carte;
}
