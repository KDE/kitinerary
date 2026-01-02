/*
   SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
   SPDX-License-Identifier: LGPL-2.0-or-later
*/

function parseOnePdfTicket(text) {
    let res = JsonLd.newTrainReservation();
    const header = text.match(/(.*)\n +(\d{6})\n(.*)/);
    if (!header)
        return;
    res.reservedTicket.ticketToken = 'qrcode:' + header[2];
    res.reservedTicket.name = header[1] + header[3];
    const trip = text.match(/(.*)  +(\d\d:\d\d) +(.*)  +(\d\d:\d\d)\n+.* (\d{1,5})  +.*(\d\d\.\d\d.\d{4})/);
    res.reservationFor.departureStation.name = trip[1];
    res.reservationFor.arrivalStation.name = trip[3];
    res.reservationFor.arrivalTime = JsonLd.toDateTime(trip[6] + ' ' + trip[4], 'dd.MM.yyyy hh:mm', 'lv');
    res.reservationFor.departureTime = JsonLd.toDateTime(trip[6] + ' ' + trip[2], 'dd.MM.yyyy hh:mm', 'lv');
    res.reservationFor.trainNumber = trip[5];
    ExtractorEngine.extractPrice(text, res);
    return res;
}

function parseOnePdfTicketVivi(text) {
    let res = JsonLd.newTrainReservation();
    const data = text.match(/(\S.*\S)\n.*\n+ *(\S.*\S)  +(\S.*\S)  +(\d+)\n.*\n+ *(\d{2}:\d{2})  +(\d{2}:\d{2})  +(\d{2}\.\d{2}\.\d{4})/);
    if (!data)
        return;
    res.reservedTicket.name = data[1];
    res.reservationFor.departureStation.name = data[2];
    res.reservationFor.arrivalStation.name = data[3];
    res.reservationFor.arrivalTime = JsonLd.toDateTime(data[7] + ' ' + data[6], 'dd.MM.yyyy hh:mm', 'lv');
    res.reservationFor.departureTime = JsonLd.toDateTime(data[7] + ' ' + data[5], 'dd.MM.yyyy hh:mm', 'lv');
    res.reservationFor.trainNumber = data[4];
    res.reservedTicket.ticketToken = 'qrcode:' + text.match(/\b(\d{6})\b/)[1];
    ExtractorEngine.extractPrice(text, res);
    return res;
}

function parsePdfTicket(pdf, node, triggerNode) {
    const page = pdf.pages[triggerNode.location];
    if (page.width < 210) {
        return parseOnePdfTicket(page.text);
    } else {
        return [parseOnePdfTicket(page.textInRect(0.0, 0.0, 0.5, 1.0)),
                parseOnePdfTicket(page.textInRect(0.5, 0.0, 1.0, 1.0)),
                parseOnePdfTicketVivi(page.textInRect(0.0, 0.0, 1.0, 0.5)),
                parseOnePdfTicketVivi(page.textInRect(0.0, 0.5, 1.0, 1.0))]
    }
}
