/*
   SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
   SPDX-License-Identifier: LGPL-2.0-or-later
*/

// see https://community.kde.org/KDE_PIM/KItinerary/Renfe_Barcodes#Common_Spanish_Ticket_Barcode
function parseBarcode(content) {
    let res = JsonLd.newTrainReservation();
    res.reservedTicket.ticketToken = 'azteccode:' + content;
    res.reservedTicket.ticketNumber = content.substr(0, 13);
    res.reservationFor.provider.identifier = 'uic:' + content.substr(13, 5);
    res.reservationFor.trainNumber = content.substr(18, 5);
    res.reservationFor.departureTime = JsonLd.toDateTime(content.substr(23, 15), 'dd/MM/yyyyhh:mm', 'es');
    res.reservationFor.departureStation.identifier = 'uic:71' + content.substr(40, 5);
    res.reservationFor.departureStation.name =  content.substr(38, 7);
    res.reservationFor.arrivalStation.identifier = 'uic:71' + content.substr(47, 5);
    res.reservationFor.arrivalStation.name = content.substr(45, 7);
    res.reservedTicket.ticketedSeat.seatSection = content.substr(52, 3);
    res.reservedTicket.ticketedSeat.seatNumber = content.substr(55, 3);

    if (content.substr(13, 5) == "01071") {
        res.reservationNumber = content.substr(143, 6);
    }
    return res;
}

function parsePdf(pdf, node, triggerNode) {
    const page = pdf.pages[triggerNode.location];
    let res = triggerNode.result[0];

    const topLeft = page.textInRect(0.0, 0.0, 0.5, 0.5);
    const stations = topLeft.match(/\n(.*)\n\d\d:\d\d\n(.*)\n(\d\d:\d\d)/);
    if (stations) {
        res.reservationFor.departureStation.name = stations[1];
        res.reservationFor.arrivalStation.name = stations[2];
        res.reservationFor.arrivalTime = JsonLd.toDateTime(stations[3], "hh:mm", "es");
    } else {
        const stationsV2 = topLeft.match(/\n* \d\d:\d\d\n(.*)\n *(\d\d:\d\d)\n(.*)/);
        res.reservationFor.departureStation.name = stationsV2[1];
        res.reservationFor.arrivalStation.name = stationsV2[3];
        res.reservationFor.arrivalTime = JsonLd.toDateTime(stationsV2[2], "hh:mm", "es");
    }

    const topRight = page.textInRect(0.5, 0.0, 1.0, 0.5);
    const name = page.text.match(/^ *Localizador: .*\n(.*)\n/);
    if (name)
        res.underName.name = name[1];
    else
        res.underName.name = topRight.match(/^(.*)\n/)[1];
    res.reservationNumber = topRight.match(/[: ] +([A-Z0-9]{6})\n/)[1];

    const price = page.text.match(/Precio.*/);
    if (price)
        ExtractorEngine.extractPrice(price[0], res);
    else
        ExtractorEngine.extractPrice(page.text, res);
    return res;
}
