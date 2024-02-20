/*
   SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

// see https://community.kde.org/KDE_PIM/KItinerary/Trenitalia_Barcode
function parseSsb(ssb, node) {
    let res = JsonLd.newTrainReservation();
    res.reservationFor.departureStation.name = "" + (ssb.departureStationNum % 10000000);
    res.reservationFor.departureStation.identifier = "uic:" + (ssb.departureStationNum % 10000000);
    if (ssb.departureStationNum != ssb.arrivalStationNum) {
        res.reservationFor.arrivalStation.name = "" + (ssb.arrivalStationNum % 10000000);
        res.reservationFor.arrivalStation.identifier = "uic:" + (ssb.arrivalStationNum % 10000000);
    }
    res.reservationFor.provider.identifier = "uic:" + ssb.issuerCode;
    if (ssb.customerNumber > 0) {
        res.programMembershipUsed.membershipNumber = ssb.customerNumber;
        res.programMembershipUsed.programName = "CartaFRECCIA";
    }

    if (ssb.firstDayOfValidityDay == ssb.lastDayOfValidityDay) {
        res.reservationFor.departureDay = ssb.firstDayOfValidity(node.contextDateTime)
    }

    res.reservationFor.trainNumber = ssb.readNumber(22*8 + 2, 16);

    const seatNum = ssb.readNumber(31*8 + 2, 7);
    const seatCol = ssb.readNumber(32*8 + 3, 4);
    if (seatNum > 0 && (seatNum != 99 || seatCol != 9)) {
        res.reservedTicket.ticketedSeat.seatNumber = "" + seatNum;
        if (seatCol > 0) {
            res.reservedTicket.ticketedSeat.seatNumber += seatCol.toString(16).toUpperCase();
        }

        res.reservedTicket.ticketedSeat.seatSection = ssb.readNumber(30*8 +6, 4)
    }

    res.reservationNumber = ssb.readString(33*8 + 6, 6);
    if (res.reservationNumber === '000000') {
        res.reservationNumber = ssb.readNumber(58*8 + 4, 32);
    }

    res.reservedTicket.ticketToken = "aztecbin:" + ByteArray.toBase64(ssb.rawData);
    return res;
}

function parsePdf(pdf, node, triggerNode) {
    let reservations = [];
    const page = pdf.pages[0];
    const text = page.text;

    var res = JsonLd.newTrainReservation();
    var pnr = text.match(/PNR: (\S+)/);
    if (pnr) {
        res.reservationNumber = pnr[1];
    }

    const leftHeaderText = page.textInRect(0.0, 0.15, 0.33, 0.25);
    const midHeaderText = page.textInRect(0.33, 0.15, 0.65, 0.25);
    const rightHeaderText = page.textInRect(0.65, 0.15, 1.0, 0.25);

    const train = rightHeaderText.match(/(?:Train|Treno|Zug)(?:\/Train)?:[ \n](.*)\n/);
    res.reservationFor.trainNumber = train[1];

    const departure_time = leftHeaderText.match(/(\d{2}:\d{2}) - (\d{2}\/\d{2}\/\d{4})/)
    const arrival_time = midHeaderText.match(/(\d{2}:\d{2}) - (\d{2}\/\d{2}\/\d{4})/)
    res.reservationFor.departureTime = JsonLd.toDateTime(departure_time[2] + departure_time[1], "dd/MM/yyyyhh:mm", "it");
    res.reservationFor.arrivalTime = JsonLd.toDateTime(arrival_time[2] + arrival_time[1], "dd/MM/yyyyhh:mm", "it");

    const dep = leftHeaderText.match(/(?:Stazione di Partenza|Departure station|Abfahrtsbahnhof|Gare de départ)(?:\/From)?\n+(.*)\n/);
    res.reservationFor.departureStation.name = dep[1];
    const arr = midHeaderText.match(/(?:Stazione di Arrivo|Arrival station|Ankunft Bahnhof|Gare d'arrivée)(?:\/To)?\n+(.*)\n/);
    res.reservationFor.arrivalStation.name = arr[1];

    const passengerColumn = page.textInRect(0.0, 0.3, 0.27, 1.0);
    const passengers = passengerColumn.match(/(?:Passenger Name|Nome Passeggero|Nom du Passager)/);
    var offset = 0;
    let seatOffset = 0;

    for (let j = 0; j < passengers.length; j++) {
        let personalRes = JsonLd.clone(res);
        var name = passengerColumn.substr(offset).match(/(?:Passenger Name|Nome Passeggero|Nom du Passager)(?:\/Passenger\n *name)?.*\n(?:    .*\n)* *((?:\w+|\-\-).*?)(?:  |\n)/);
        offset += name.index + name[0].length;
        if (name[1] !== "--") {
            personalRes.underName.name = name[1];
        } else {
            personalRes.underName.name = "Passenger " + (j + 1);
        }

        var coach = text.match(/(?:Coaches|Carrozza|Wagen|Voiture)(?:\/Coach)?: +(\S+)/);
        if (coach) {
            personalRes.reservedTicket.ticketedSeat.seatSection = coach[1];
        }

        // fallback seat parsing for unparsable ERA FCB tickets
        const seat = page.text.substr(seatOffset).match(/(\d+) +(\d+[A-F]?) +([A-Z0-9]{6})/);
        if (seat) {
            seatOffset += seat.index + seat[0].length;
            if (personalRes.reservedTicket.ticketedSeat.seatSection == seat[1] && !personalRes.reservedTicket.ticketedSeat.seatNumber) {
                personalRes.reservedTicket.ticketedSeat.seatNumber = seat[2];
            }
        }

        ExtractorEngine.extractPrice(text, personalRes);
        reservations.push(personalRes);
    }

    return reservations;
}

function parseEvent(event)
{
    let res = JsonLd.newTrainReservation();
    res.reservationFor.departureTime = JsonLd.readQDateTime(event, 'dtStart');
    res.reservationFor.arrivalTime = JsonLd.readQDateTime(event, 'dtEnd');
    res.reservationFor.departureStation.name = event.location;
    res.reservationFor.arrivalStation.name = event.description.match(event.location + "-(.*?);")[1];
    res.reservationFor.trainNumber = event.description.match(/(?:Train|Treno|Zug): (.*?),/)[1];

    const code = event.description.match(/(?:pnr code|codice pnr) ([A-Z0-9]{6})\b/);
    if (code) {
        res.reservationNumber = code[1];
    }

    const seat = event.description.match(/(?:Coach|Carrozza||Wagen|Voiture) (.*?), (?:Position|Posti) (.*?)[;\b]/);
    if (seat) {
        res.reservedTicket.ticketedSeat.seatSection = seat[1];
        res.reservedTicket.ticketedSeat.seatNumber = seat[2];
    }

    return res;
}

function parseHtml(html)
{
    let reservations = [];

    // Informations are organized in a different HTML hierarchy when rappresenting
    // - a single trip for a single person
    // - many trips for a single person
    // - many trip for many people
    // Here we try to move the text offset between the different blocks, using the
    // total number of trains mentioned within the whole document as a reference
    const blocks = html.eval("//b[text()='Treno:']");
    const totalTrains = blocks.length;
    let fullText = blocks[0].parent.parent.parent.recursiveContent;
    let offset = 0;

    // All trips involved in a booking are managed together, with a single
    // "check-in" link and a single panel to modify. For convenience, those
    // links are attached to each trip
    const checkinUrl = html.eval('//a[contains(@href, "/self-check-in")]')[0].attribute('href');
    const modifyReservationUrl = html.eval('//a[contains(@href, "/user-area/purchase/ticketDetail")]')[0].attribute('href');

    for(let j = 0; j < totalTrains; j++) {
        let currentText = fullText.substr(offset);
        let train = currentText.match(/(?:Train|Treno|Zug): (.*?) *del *([0-9\/]*)/);

        if (train == null) {
            offset = 0;
            fullText = blocks[j].parent.parent.parent.recursiveContent;
            currentText = fullText;
            train = currentText.match(/(?:Train|Treno|Zug): (.*?) *del *([0-9\/]*)/);
        }

        const date = train[2].trim();
        const start = currentText.match(/(?:Partenza) ?: (.*?)\(Ore: ([0-9:]*)\)/);
        const end = currentText.match(/(?:Arrivo) ?: (.*?)\(Ore: ([0-9:]*)\)/);
        const name = currentText.match(/(?:Nome Viaggiatore) ?: ?(.*?);/);
        offset += end.index + end[1].length + end[2].length;

        let res = JsonLd.newTrainReservation();
        res.reservationFor.trainNumber = train[1];
        res.reservationFor.departureStation.name = start[1];
        res.reservationFor.arrivalStation.name = end[1];
        res.reservationFor.departureTime = JsonLd.toDateTime(date + start[2], "dd/MM/yyyyhh:mm", "it");
        res.reservationFor.arrivalTime = JsonLd.toDateTime(date + end[2], "dd/MM/yyyyhh:mm", "it");

        // Tickets for trains not directly operated by Trenitalia (e.g. Trenord) are
        // often not assigned to a specific name, so this value is not always explicit
        if (name) {
            res.underName.name = name[1];
        }

        res.checkinUrl = checkinUrl;
        res.modifyReservationUrl = modifyReservationUrl;
        ExtractorEngine.extractPrice(currentText, res);

        reservations.push(res);
    }

    return reservations;
}
