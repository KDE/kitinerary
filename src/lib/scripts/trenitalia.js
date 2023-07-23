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
    if (seatNum > 0) {
        res.reservedTicket.ticketedSeat.seatNumber = "" + seatNum;
        const seatCol = ssb.readNumber(32*8 + 3, 4);
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
    const page = pdf.pages[triggerNode.location];
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

    const barcodes = node.findChildNodes({ scope: "Descendants", mimeType: "internal/era-ssb", field: "issuerCode", match: "83" }).concat(node.findChildNodes({ scope: "Descendants", mimeType: "internal/uic9183", field: "carrierId", match: "83" }));
    var offset = 0;
    const passengerColumn = page.textInRect(0.0, 0.3, 0.27, 1.0);
    let seatOffset = 0;
    for (let j = 0; j < barcodes.length; ++j) {
        if (barcodes[j].location != triggerNode.location) {
            continue;
        }
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

        if (barcodes[j].result[0]['@type'] == 'TrainReservation') {
            personalRes = JsonLd.apply(barcodes[j].result[0], personalRes);
        } else {
            personalRes.reservedTicket = JsonLd.apply(barcodes[j].result[0], personalRes.reservedTicket);
        }

        // fallback seat parsing for unparsable ERA FCB tickets
        const seat = page.text.substr(seatOffset).match(/(\d+) +(\d+[A-F]?) +([A-Z0-9]{6})/);
        if (seat) {
            seatOffset += seat.index + seat[0].length;
            if (personalRes.reservedTicket.ticketedSeat.seatSection == seat[1] && !personalRes.reservedTicket.ticketedSeat.seatNumber) {
                personalRes.reservedTicket.ticketedSeat.seatNumber = seat[2];
            }
        }

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
    res.reservationFor.trainNumber = event.description.match(/Train: (.*?),/)[1];
    res.reservationNumber = event.description.match(/pnr code ([A-Z0-9]{6})\b/)[1];
    const seat = event.description.match(/Coach (.*?), Position (.*?)[;\b]/);
    if (seat) {
        res.reservedTicket.ticketedSeat.seatSection = seat[1];
        res.reservedTicket.ticketedSeat.seatNumber = seat[2];
    }
    return res;
}
