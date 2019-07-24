/*
   Copyright (c) 2019 Volker Krause <vkrause@kde.org>

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

        var train = text.match(/(?:Train|Treno): (.*)\n/);
        if (!train) {
            break;
        }
        res.reservationFor.trainNumber = train[1];

        var times = text.match(/(?:Hours|Ore(?:\/Time)?) (\d{2}:\d{2}) - (\d{2}\/\d{2}\/\d{4}) +(?:Hours|Ore(?:\/Time)?) (\d{2}:\d{2}) - (\d{2}\/\d{2}\/\d{4})/)
        res.reservationFor.departureTime = JsonLd.toDateTime(times[2] + times[1], "dd/MM/yyyyhh:mm", "it");
        res.reservationFor.arrivalTime = JsonLd.toDateTime(times[4] + times[3], "dd/MM/yyyyhh:mm", "it");

        var header = text.match(/(?:Stazione di Arrivo|Arrival station)/);
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
            var bitArray = Barcode.toBitArray(barcode);

            var depUic = bitArray.readNumberMSB(14*8 + 4, 24);
            var arrUic = bitArray.readNumberMSB(18*8 + 3, 24);
            if (depUic != arrUic) {
                if (bitArray.readNumberMSB(14*8, 4) == 0) {
                    personalRes.reservationFor.departureStation.identifier = "uic:" + depUic;
                }
                if (bitArray.readNumberMSB(17*8 + 7, 4) == 0) {
                    personalRes.reservationFor.arrivalStation.identifier = "uic:" + arrUic;
                }
            }

            var seatNum = bitArray.readNumberMSB(31*8 + 2, 7);
            if (seatNum > 0) {
                personalRes.reservedTicket.ticketedSeat.seatNumber = "" + seatNum;
                var seatCol = bitArray.readNumberMSB(32*8 + 3, 4);
                if (seatCol > 0) {
                    personalRes.reservedTicket.ticketedSeat.seatNumber += seatCol.toString(16).toUpperCase();
                }

                var coach = text.match(/(?:Coaches|Carrozza(?:\/Coach)?): +(\S+)/);
                personalRes.reservedTicket.ticketedSeat.seatSection = coach[1];
            }

            reservations.push(personalRes);
        }
    }

    return reservations;
}
