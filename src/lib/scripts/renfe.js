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
    // 2x unknown number
    // 1x unknown number
    // 6x "localizador" ~ PNR?
    // ".." (optional)
    // 5x "CombinadoCercanias" (optional)

    if (barcode.trim().length > 56)
        return null;

    // trim not set combined commuter section, so merging with a document that has that set works properly
    if (barcode.substr(49, 7) == '..00000')
        barcode = barcode.substr(0, 49);

    var res = JsonLd.newTrainReservation();
    res.reservedTicket.ticketToken = "qrCode:" + barcode.trim();
    res.reservationFor.trainNumber = barcode.substr(29, 5);
    res.reservedTicket.ticketedSeat.seatSection = barcode.substr(34, 3);
    res.reservedTicket.ticketedSeat.seatNumber = barcode.substr(37, 3);
    res.reservationNumber = barcode.substr(43, 6);
    res.reservationFor.departureStation.identifier = "uic:71" + barcode.substr(13, 5);
    res.reservationFor.arrivalStation.identifier = "uic:71" + barcode.substr(18, 5);
    res.reservationFor.departureDay = JsonLd.toDateTime(barcode.substr(23, 6), 'ddMMyy', 'es');

    res.reservationFor.departureStation.name = barcode.substr(13, 5);
    res.reservationFor.arrivalStation.name = barcode.substr(18, 5);
    return res;
}

function parseLeg(text, baseRes)
{
    let res = JsonLd.newTrainReservation();

    const dep = text.match(/(?:Salida|Origen:) +(.*?) {2,}([\d\/]+) +(\d\d[:\.]\d\d)/);
    res.reservationFor.departureStation.name = dep[1];
    res.reservationFor.departureTime = JsonLd.toDateTime(dep[2] + dep[3], ["dd/MM/yyyyhh:mm", "dd/MM/yyyyhh.mm"], "es");

    const arr = text.match(/(?:Llegada|Destino:)\s+(.*?) {2,}([\d\/]+) +(\d\d[:\.]\d\d)\n(?:.* )?([A-Z]+) +(\d+)/);
    res.reservationFor.arrivalStation.name = arr[1];
    res.reservationFor.arrivalTime = JsonLd.toDateTime(arr[2] + arr[3], ["dd/MM/yyyyhh:mm", "dd/MM/yyyyhh.mm"], "es");
    res.reservationFor.trainName = arr[4];

    const train = text.match(/Coche:\s+(\S+)\s+Plaza:\s+(\S+)\s+(\S.*) +(\d{1,5})/);
    if (train) {
        res.reservedTicket.ticketedSeat.seatSection = train[1];
        res.reservedTicket.ticketedSeat.seatNumber = train[2];
        res.reservationFor.trainName = train[3];
        res.reservationFor.trainNumber = train[4];
    }

    res.reservationNumber = baseRes.reservationNumber;
    res.reservationFor.provider = baseRes.reservationFor.provider;
    res.reservedTicket.ticketToken = baseRes.reservedTicket.ticketToken;
    return res;
}

function parsePdf(pdf, node, triggerNode)
{
    const res = triggerNode.result[0];
    const page = pdf.pages[triggerNode.location]
    const text = page.text;
    if (!text.match(/TRAYECTO/)) {
        return JsonLd.apply(res, parseLeg(text, res));
    }

    // multi-leg ticket
    let res1 = parseLeg(page.textInRect(0.0, 0.0, 0.5, 1.0), res);
    let res2 = parseLeg(page.textInRect(0.5, 0.0, 1.0, 1.0), res);
    res1.reservationFor.departureStation = JsonLd.apply(res.reservationFor.departureStation, res1.reservationFor.departureStation);
    res2.reservationFor.arrivalStation = JsonLd.apply(res.reservationFor.arrivalStation, res2.reservationFor.arrivalStation);
    return [res1, res2];
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
