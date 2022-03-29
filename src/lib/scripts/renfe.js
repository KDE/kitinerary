/*
   SPDX-FileCopyrightText: 2019-2022 Volker Krause <vkrause@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

function parseBarcode(barcode)
{
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

    if (barcode.length > 56)
        return null;
    var res = JsonLd.newTrainReservation();
    res.reservedTicket.ticketToken = "qrCode:" + barcode;
    res.reservationFor.trainNumber = barcode.substr(29, 5);
    res.reservedTicket.ticketedSeat.seatSection = barcode.substr(34, 3);
    res.reservedTicket.ticketedSeat.seatNumber = barcode.substr(37, 3);
    res.reservationNumber = barcode.substr(43, 6);
    res.reservationFor.departureStation.identifier = "uic:71" + barcode.substr(13, 5);
    res.reservationFor.arrivalStation.identifier = "uic:71" + barcode.substr(18, 5);
    res.reservationFor.departureDay = JsonLd.toDateTime(barcode.substr(23, 6), 'ddMMyy', 'es');
    return res;
}

function parsePdf(pdf, node, triggerNode)
{
    if (!triggerNode.content)
        return;

    var res = parseBarcode(triggerNode.content.trim());
    var text = pdf.pages[triggerNode.location].text;
    var dep = text.match(/(?:Salida|Origen:) +(.*?) {2,}([\d\/]+) +(\d\d:\d\d)/);
    res.reservationFor.departureStation.name = dep[1];
    res.reservationFor.departureTime = JsonLd.toDateTime(dep[2] + dep[3], "dd/MM/yyyyhh:mm", "es");

    var arr = text.match(/(?:Llegada|Destino:)\s+(.*?) {2,}([\d\/]+) +(\d\d:\d\d)\n *(\S+) /);
    res.reservationFor.arrivalStation.name = arr[1];
    res.reservationFor.arrivalTime = JsonLd.toDateTime(arr[2] + arr[3], "dd/MM/yyyyhh:mm", "es");
    res.reservationFor.trainName = arr[4];

    return res;
}

function parsePkPass(content)
{
    var res = parseBarcode(content.barcodes[0].message);
    res.reservationFor.provider.name = content.organizationName;

    const day = content.field['destinofecha'].value;
    res.reservationFor.departureTime = JsonLd.toDateTime(day + ' ' + content.field['boardingTime'].value, 'dd/MM/yyyy hh:mm', 'es');
    res.reservationFor.departureStation.name = content.field['boardingTime'].label;
    res.reservationFor.arrivalTime = JsonLd.toDateTime(day + ' ' + content.field['destino'].value, 'dd/MM/yyyy hh:mm', 'es');
    res.reservationFor.arrivalStation.name = content.field['destino'].label;

    res.underName.name = content.field['nombrepasajero'].value;

    if (content.field['documentos']) {
        const program = content.field['documentos'].value.match(/(.*) +: +(.*)/);
        if (program) {
            res.programMembershipUsed.name = program[1];
            res.programMembershipUsed.membershipNumber = program[2];
        }
    }

    return res;
}
