/*
   SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

function parsePdf(pdf) {
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

        var images = page.images;
        var offset = 0;
        for (var j = 0; j < images.length; ++j) {
            var barcode = Barcode.decodeAztecBinary(images[j]);
            var barcodeB64 = Barcode.toBase64(barcode);
            if (!barcodeB64)
                continue;

            var personalRes = JsonLd.clone(res);
            personalRes.reservedTicket.ticketToken = "aztecbin:" + barcodeB64;

            var name = text.substr(offset).match(/(?:Passenger Name|Nome Passeggero(?:\/Passenger\n name)?).*\n(?:    .*\n)* ?((?:\w+|\-\-).*?)(?:  |\n)/);
            offset += name.index + name[0].length;
            if (name[1] !== "--") {
                personalRes.underName.name = name[1];
            } else {
                personalRes.underName.name = "Passenger " + j;
            }

            // see https://community.kde.org/KDE_PIM/KItinerary/Trenitalia_Barcode
            var ssb = Barcode.decodeEraSsbTicket(barcode)
            personalRes.reservationFor.departureStation.identifier = "uic:" + (ssb.departureStationNum % 10000000)
            if (ssb.departureStationNum != ssb.arrivalStationNum) {
                personalRes.reservationFor.arrivalStation.identifier = "uic:" + (ssb.arrivalStationNum % 10000000)
            }

            var bitArray = Barcode.toBitArray(barcode);
            var seatNum = bitArray.readNumberMSB(31*8 + 2, 7);
            if (seatNum > 0) {
                personalRes.reservedTicket.ticketedSeat.seatNumber = "" + seatNum;
                var seatCol = bitArray.readNumberMSB(32*8 + 3, 4);
                if (seatCol > 0) {
                    personalRes.reservedTicket.ticketedSeat.seatNumber += seatCol.toString(16).toUpperCase();
                }

                var coach = text.match(/(?:Coaches|Carrozza|Wagen)(?:\/Coach)?: +(\S+)/);
                personalRes.reservedTicket.ticketedSeat.seatSection = coach[1];
            }

            reservations.push(personalRes);
        }
    }

    return reservations;
}
