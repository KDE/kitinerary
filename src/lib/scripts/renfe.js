/*
   SPDX-FileCopyrightText: 2019-2022 Volker Krause <vkrause@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

// see https://community.kde.org/KDE_PIM/KItinerary/Renfe_Barcodes#Renfe_Barcode
function parseBarcode(barcode)
{
    if (barcode.trim().length > 56)
        return null;

    // trim not set combined commuter section, so merging with a document that has that set works properly
    if (barcode.endsWith('..00000'))
        barcode = barcode.slice(0, -7);

    var res = JsonLd.newTrainReservation();
    res.reservedTicket.ticketToken = "qrCode:" + barcode.trim();
    res.reservedTicket.ticketNumber = barcode.substr(0, 13);
    res.reservationFor.trainNumber = barcode.substr(29, 5);
    res.reservedTicket.ticketedSeat.seatSection = barcode.substr(34, 3);
    res.reservedTicket.ticketedSeat.seatNumber = barcode.substr(37, 3);
    if (barcode.match(/null/))
        res.reservationNumber = barcode.slice(-6);
    else
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

    const arr = text.match(/(?:Llegada|Destino:)\s+(.*?) {2,}([\d\/]+) +(\d\d[:\.]\d\d)\n(?:.* )?([A-Z]+) *(\d+)/);
    res.reservationFor.arrivalStation.name = arr[1];
    res.reservationFor.arrivalTime = JsonLd.toDateTime(arr[2] + arr[3], ["dd/MM/yyyyhh:mm", "dd/MM/yyyyhh.mm"], "es");
    res.reservationFor.trainName = arr[4];

    const train = text.match(/Coche:\s+(\S+)\s+Plaza:\s+(\S+)\s+(\w.*?\w) *(\d{1,5})/);
    if (train) {
        res.reservedTicket.ticketedSeat.seatSection = train[1];
        res.reservedTicket.ticketedSeat.seatNumber = train[2];
        res.reservationFor.trainName = train[3];
        res.reservationFor.trainNumber = train[4];
    }

    res.reservationNumber = baseRes.reservationNumber;
    res.reservationFor.provider = baseRes.reservationFor.provider;
    res.reservedTicket.ticketToken = baseRes.reservedTicket.ticketToken;
    res.reservedTicket.ticketNumber = baseRes.reservedTicket.ticketNumber;
    res.priceCurrency = baseRes.priceCurrency;
    res.totalPrice = baseRes.totalPrice;
    res.underName = baseRes.underName;
    return res;
}

function parsePdf(pdf, node, triggerNode)
{
    const res = triggerNode.result[0];
    const page = pdf.pages[triggerNode.location]
    const text = page.text;

    const price = text.match(/TOTAL:? *(\d+,\d\d *€)/i);
    if (price)
        ExtractorEngine.extractPrice(price[1], res);
    const pas = text.match(/Localizador: [A-Z0-9]{6}(?: +CombinadoCercanias: +[A-Z0-9]{5})?\n([^\d:]+)(?:\n|  )/);
    if (pas)
        res.underName.name = pas[1];

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
    ExtractorEngine.extractPrice(content.field['precio'].value, res);
    return res;
}

function parseInternationalPdf(pdf, node, uicNode)
{
    const renfeNode = node.findChildNodes({ scope: "Descendants", mimeType: "text/plain", match: "^\\d{25}/\\d{2}/\\d{6}:\\d{16}" }).find(n => n.location === uicNode.location);

    let res = JsonLd.apply(renfeNode.result[0], uicNode.result[0]);
    res.reservationFor.departureStation.identifier = undefined;
    res.reservationFor.arrivalStation.identifier = undefined;
    res.reservationNumber = renfeNode.result[0].reservationNumber;
    let renfeRes = JsonLd.clone(res);
    renfeRes.reservedTicket.ticketToken = renfeNode.result[0].reservedTicket.ticketToken;
    renfeRes.reservedTicket.name = renfeNode.result[0].reservedTicket.name;
    return [renfeRes, res];
}
