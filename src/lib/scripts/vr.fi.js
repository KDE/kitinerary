/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

// see https://community.kde.org/KDE_PIM/KItinerary/vr.fi_Barcode
function readStationCode(bitarray, offset)
{
    var s = "";
    for (var i = 0; i < 5; ++i) {
        var n = bitarray.readNumberMSB(offset + i * 6, 6);
        if (n != 36)
            s += String.fromCharCode(n + 55);
    }
    return s;
}

function parseTicket(pdf) {
    var res = JsonLd.newTrainReservation();
    var bitarray = Barcode.toBitArray(Context.barcode);

    var trainNum = bitarray.readNumberMSB(22 * 8, 14) + "";
    if (trainNum == 0)
        return null; // TODO this misses bus legs!

    var text = pdf.pages[Context.pdfPageNumber].text;
    var trip = text.match("(.*) - (.*)\n.*(\\d{4}).*?(\\d{2}:\\d{2}).*?(\\d{2}:\\d{2})\n(.*?" + trainNum + ")");
    res.reservationFor.trainNumber = trip[6];

    var departureDay = bitarray.readNumberMSB(4 * 8 + 7, 9) - 1;
    var day = new Date(0);
    day.setYear(trip[3]);
    day.setTime(day.getTime() + departureDay * 24 * 60 * 60 * 1000);
    res.reservationFor.departureTime = JsonLd.toDateTime(day.getFullYear() + '-' + (day.getMonth() + 1) + '-' + day.getDate() + '-' + trip[4], "yyyy-M-d-hh:mm", "en");
    res.reservationFor.arrivalTime = JsonLd.toDateTime(day.getFullYear() + '-' + (day.getMonth() + 1) + '-' + day.getDate() + '-' + trip[5], "yyyy-M-d-hh:mm", "en");

    var coachNumber = bitarray.readNumberMSB(30 * 8, 6);
    if (coachNumber > 0) {
        res.reservedTicket.ticketedSeat.seatSection = coachNumber + "";
        res.reservedTicket.ticketedSeat.seatNumber = bitarray.readNumberMSB(30 * 8 + 6, 7) + "";
    }

    // for station codes see: https://rata.digitraffic.fi/api/v1/metadata/stations
    res.reservationFor.departureStation.name = trip[1];
    res.reservationFor.departureStation.identifier = "vrfi:" + readStationCode(bitarray, 13*8 + 2);
    res.reservationFor.arrivalStation.name = trip[2];
    res.reservationFor.arrivalStation.identifier = "vrfi:" + readStationCode(bitarray, 17*8 + 1);

    res.reservedTicket.ticketToken = "aztectbin:" + Barcode.toBase64(Context.barcode);
    return res;
}
