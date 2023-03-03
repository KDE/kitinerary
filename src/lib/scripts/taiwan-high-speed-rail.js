/*
   SPDX-FileCopyrightText: 2023 Luca Weiss <luca@z3ntu.xyz>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

// List of stations present in Taiwan High Speed Rail network.
// "code" value matches wikidata "station code" (P296)
const stations = {
    1: {"code": "NAG／1", "name": "南港", "nameEn": "Nangang"},
    2: {"code": "TPE／2", "name": "台北", "nameEn": "Taipei"},
    3: {"code": "BAQ／3", "name": "板橋", "nameEn": "Banqiao"},
    4: {"code": "TAY／4", "name": "桃園", "nameEn": "Taoyuan"},
    5: {"code": "HSC／5", "name": "新竹", "nameEn": "Hsinchu"},
    6: {"code": "MIL／6", "name": "苗栗", "nameEn": "Miaoli"},
    7: {"code": "TAC／7", "name": "台中", "nameEn": "Taichung"},
    8: {"code": "CHH／8", "name": "彰化", "nameEn": "Changhua"},
    9: {"code": "YUL／9", "name": "雲林", "nameEn": "Yunlin"},
    10: {"code": "CHY／10", "name": "嘉義", "nameEn": "Chiayi"},
    11: {"code": "TAN／11", "name": "台南", "nameEn": "Tainan"},
    12: {"code": "ZUY／12", "name": "左營", "nameEn": "Zuoying"},
}

function main(text) {
    /*
     * Unknown parts of data:
     * 21-23: 000, possibly space for larger train numbers?
     * 43-44: 00
     * 60-74: various values, unknown
     * 94-99: various values, unknown
     * 100-105: "INTIRS" or empty (000000), ticket type?
     * 106-109: 0000
     * 110-117: date of travel?
     * 118-123: various values, unknown
    */
    const ticketNumber = text.substr(0, 13);
    const reservationCode = text.substr(13, 8);
    const trainNumber = parseInt(text.substr(24, 4));
    const departureStationNr = parseInt(text.substr(28, 3));
    const departureDateTime = JsonLd.toDateTime(text.substr(31, 12), "yyyyMMddhhmm", "zh");
    const arrivalStationNr = parseInt(text.substr(45, 3));
    const arrivalDateTime = JsonLd.toDateTime(text.substr(48, 12), "yyyyMMddhhmm", "zh");
    const seatData = text.substr(75, 9);
    const price = parseInt(text.substr(84, 10));

    const departureStation = stations[departureStationNr];
    const arrivalStation = stations[arrivalStationNr];

    // seatData is 3 numbers each: car, row & seat (A-E but represented as 1-5)
    const seatCar = parseInt(seatData.substr(0, 3));
    const seatRow = parseInt(seatData.substr(3, 3));
    const seatNumber = parseInt(seatData.substr(6, 3));
    const seatLetter = String.fromCharCode(65 + (seatNumber-1));
    const seat = seatRow + seatLetter;

    var res = JsonLd.newTrainReservation();
    res.reservationFor.departureStation.name = departureStation["name"] + " (" + departureStation["nameEn"] + ")";
    res.reservationFor.departureStation.identifier = departureStation["code"];
    res.reservationFor.departureTime = departureDateTime;
    res.reservationFor.arrivalStation.name = arrivalStation["name"] + " (" + arrivalStation["nameEn"] + ")";
    res.reservationFor.arrivalStation.identifier = arrivalStation["code"];
    res.reservationFor.arrivalTime = arrivalDateTime;
    res.reservationFor.trainNumber = trainNumber;
    res.reservedTicket.ticketedSeat.seatSection = seatCar;
    res.reservedTicket.ticketedSeat.seatNumber = seat;
    res.reservedTicket.ticketToken = "qrCode:" + text;
    res.totalPrice = price;
    res.priceCurrency = "TWD";

    return res;
}
