/*
   SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

function parsePdf(pdf, node, triggerNode)
{
    if (!triggerNode.content)
        return;

    // barcode content:
    // 13x ticket number
    // 5x Renfe departure station id
    // 5x Renfe arrival station id
    // 6x departure(?) date: ddMMyy
    // 5x train number
    // 3x coach number
    // 3x seat number
    // 3x unknown number
    // 6x "localizador" ~ PNR?
    // ".." (optional)
    // 5x "CombinadoCercanias" (optional)

    var barcode = triggerNode.content;
    var res = JsonLd.newTrainReservation();
    res.reservedTicket.ticketToken = "dataMatrix:" + barcode;
    res.reservationFor.trainNumber = barcode.substr(29, 5);
    res.reservedTicket.ticketedSeat.seatSection = barcode.substr(34, 3);
    res.reservedTicket.ticketedSeat.seatNumber = barcode.substr(37, 3);
    res.reservationNumber = barcode.substr(43);

    var text = pdf.pages[triggerNode.location].text;
    var dep = text.match(/Salida +(.*?) {2,}([\d\/]+) +(\d\d:\d\d)/);
    res.reservationFor.departureStation.name = dep[1];
    res.reservationFor.departureStation.identifier = "uic:71" + barcode.substr(13, 5);
    res.reservationFor.departureTime = JsonLd.toDateTime(dep[2] + dep[3], "dd/MM/yyyyhh:mm", "es");

    var arr = text.match(/Llegada\s+(.*?) {2,}([\d\/]+) +(\d\d:\d\d)\n *(\S+) /);
    res.reservationFor.arrivalStation.name = arr[1];
    res.reservationFor.arrivalStation.identifier = "uic:71" + barcode.substr(18, 5);
    res.reservationFor.arrivalTime = JsonLd.toDateTime(arr[2] + arr[3], "dd/MM/yyyyhh:mm", "es");
    res.reservationFor.trainName = arr[4];

    return res;
}
