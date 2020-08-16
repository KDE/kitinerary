/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

// see https://community.kde.org/KDE_PIM/KItinerary/Thalys_Barcode
function readEncodedString(bitArray, startIdx, size)
{
    var id = "";
    for (var i = 0; i < size; ++i) {
        id += String.fromCharCode(bitArray.readNumberMSB(startIdx + i * 6, 6) + 32);
    }
    return id;
}

function parseReservation(html) {
    var res = JsonLd.newTrainReservation();

    var subtitle = html.eval('//table[@class="subtitle"]');
    var ref = subtitle[0].recursiveContent.match(/(\d{2}.\d{2}.\d{4})[\s\S]*([A-Z0-9]{6})/);
    res.reservationNumber = ref[2];

    var schedule = html.eval('//table[@class="schedule"]')[0].eval(".//tr");
    var stations = schedule[1].recursiveContent.match(/(.*)\n.*\n(.*)/);
    res.reservationFor.departureStation.name = stations[1];
    res.reservationFor.arrivalStation.name = stations[2];

    var times = schedule[2].recursiveContent.match(/(\d{2}:\d{2})[\s\S]*(\d{2}:\d{2})/);
    res.reservationFor.departureTime = JsonLd.toDateTime(ref[1] + times[1], "dd/MM/yyyyhh:mm", "en");
    res.reservationFor.arrivalTime = JsonLd.toDateTime(ref[1] + times[2], "dd/MM/yyyyhh:mm", "en");

    var detailsElem = html.eval('//table[@class="detailtrain"]')[0];
    var details = detailsElem.recursiveContent.match(/(\d{4})\n[\s\S]*?(\d{1})\n[\s\S]*?(\d{1,2})\n[\s\S]*?(\d{1,3})/);
    res.reservationFor.trainNumber = "THA " + details[1];
    res.reservedTicket.ticketedSeat.seatingType = details[2];
    res.reservedTicket.ticketedSeat.seatSection = details[3];
    res.reservedTicket.ticketedSeat.seatNumber = details[4];

    var tokenElem = html.eval('//table[@class="qrcode"]//img')[0];
    var token = tokenElem.attribute("src").match(/barcode\/tAZTEC\/.*\/nBinary\/v(.*)\/barcode.gif/);
    res.reservedTicket.ticketToken = "aztecbin:" + token[1];

    var passengerElem = html.eval('//table[@class="passengername"]')[0];
    var name = passengerElem.recursiveContent.match(/\n(.*)/);
    res.underName.name = name[1];

    var bitArray = Barcode.toBitArray(Barcode.fromBase64(token[1]));
    res.reservationFor.departureStation.identifier = "benerail:" + readEncodedString(bitArray, 18 * 8 + 4, 5);
    res.reservationFor.arrivalStation.identifier = "benerail:" + readEncodedString(bitArray, 22 * 8 + 2, 5);

    return res;
}
