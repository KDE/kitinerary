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

function parsePdf(pdf, node) {
    var reservations = new Array();

    for (var i = 0; i < pdf.pageCount; ++i) {
        var page = pdf.pages[i];
        var text = page.text;

        var res = JsonLd.newTrainReservation();
        var pnr = text.match(/PNR: (\S+)/);
        if (pnr) {
            res.reservationNumber = pnr[1];
        }

        var train = text.match(/(?:Train|Treno|Zug): (.*)\n/);
        if (!train) {
            break;
        }
        res.reservationFor.trainNumber = train[1];

        var departure_time = text.match(/(?:Hours|Ore|Stunden)(?:\/Time)? (\d{2}:\d{2}) - (\d{2}\/\d{2}\/\d{4})/)
        var arrival_time = text.substr(departure_time.index + departure_time[0].length).match(/(?:Hours|Ore|Stunden)(?:\/Time)? (\d{2}:\d{2}) - (\d{2}\/\d{2}\/\d{4})/)
        res.reservationFor.departureTime = JsonLd.toDateTime(departure_time[2] + departure_time[1], "dd/MM/yyyyhh:mm", "it");
        res.reservationFor.arrivalTime = JsonLd.toDateTime(arrival_time[2] + arrival_time[1], "dd/MM/yyyyhh:mm", "it");

        var header = text.match(/(?:Stazione di Arrivo|Arrival station|Ankunft Bahnhof)/);
        var dest = text.substr(header.index + header[0].length).match(/\n *((?:\w+\.? )*\w+\.?)  +((?:\w+\.? )*\w+\.?)(?:  |\n)/);
        res.reservationFor.departureStation.name = dest[1];
        res.reservationFor.arrivalStation.name = dest[2];

        const barcodes = node.findChildNodes({ scope: "Descendants", mimeType: "internal/era-ssb", field: "issuerCode", match: "83" });
        var offset = 0;
        for (let j = 0; j < barcodes.length; ++j) {
            if (barcodes[j].location != i) {
                continue;
            }
            let personalRes = JsonLd.clone(res);
            var name = text.substr(offset).match(/(?:Passenger Name|Nome Passeggero(?:\/Passenger\n name)?).*\n(?:    .*\n)* ?((?:\w+|\-\-).*?)(?:  |\n)/);
            offset += name.index + name[0].length;
            if (name[1] !== "--") {
                personalRes.underName.name = name[1];
            } else {
                personalRes.underName.name = "Passenger " + (j + 1);
            }

            var coach = text.match(/(?:Coaches|Carrozza|Wagen)(?:\/Coach)?: +(\S+)/);
            if (coach) {
                personalRes.reservedTicket.ticketedSeat.seatSection = coach[1];
            }

            personalRes = JsonLd.apply(barcodes[j].result[0], personalRes);
            reservations.push(personalRes);
        }
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
