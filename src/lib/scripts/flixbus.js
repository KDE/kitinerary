/*
   SPDX-FileCopyrightText: 2019-2022 Volker Krause <vkrause@kde.org>
   SPDX-License-Identifier: LGPL-2.0-or-later
*/

// convert bus reservations to train reservations when needed
function fixTransportMode(res) {
    if (res.reservationFor.departureBusStop.name.endsWith(" (FlixTrain)") && res.reservationFor.arrivalBusStop.name.endsWith(" (FlixTrain)")) {
        res = JsonLd.busToTrainReservation(res);
        res.reservationFor.departureStation.name = res.reservationFor.departureStation.name.substr(0, res.reservationFor.departureStation.name.length - 11);
        res.reservationFor.arrivalStation.name = res.reservationFor.arrivalStation.name.substr(0, res.reservationFor.arrivalStation.name.length - 11);
    }
    return res;
}

function main(content, node) {
    // convert QR download links into the actual QR codes
    var res = node.result;
    for (var i = 0; i < res.length; ++i) {
        var ticketToken = res[i].reservedTicket.ticketToken;
        res[i].reservedTicket.ticketToken = ticketToken.replace(/^https?:\/\/api\.(?:flixbus|meinfernbus)\..{2,3}\/qrcode\/(..)\//, "qrCode:https://shop.flixbus.$1/pdfqr/");

        // their schema.org annotations also claim train trips are bus trips, fix that
        res[i] = fixTransportMode(res[i]);
    }
    return res;
}

function parseDate(year, baseDate, overrideDate, time)
{
    baseDate = baseDate.replace('.', '');
    if (overrideDate)
        overrideDate = overrideDate.replace('.', '');
    const s = (overrideDate ? overrideDate.trim() : baseDate) + ' ' + year + ' ' + time;
    return JsonLd.toDateTime(s, 'd MMM yyyy hh:mm', ['en', 'fr', 'pl', 'nl', 'de']);
}

function parseLocation(place, addr1, addr2, links)
{
    if (!addr1)
        return;
    if (addr2)
        addr1 += ', ' + addr2;
    const idx = addr1.lastIndexOf(',');
    place.address.streetAddress = addr1.substring(0, idx);
    place.address.addressLocality = addr1.substr(idx + 1);
    place.geo = JsonLd.toGeoCoordinates(links.shift().url);
}

function parsePdfTicket(pdf, node, triggerNode)
{
    const page = pdf.pages[triggerNode.location];
    const text = page.textInRect(0.0, 0.05, 0.5, 0.5);
    const links = page.linksInRect(0.0, 0.0, 0.5, 0.5);
    const resNum = triggerNode.content.match(/pdfqr\/(\d+)\//)[1];
    const date = text.match(/^\S+,? (\d+\.? \S+) (\d{4})\n/);

    const timeColumn = page.textInRect(0.0, 0.1, 0.125, 0.5);
    const stationColumn = page.textInRect(0.125, 0.1, 0.5, 0.5);

    let idxTime = 0;
    let idxStations = 0;
    let reservations = [];
    while (true) {
        const times = timeColumn.substr(idxTime).match(/(\d\d:\d\d)\n([^:]*?\n)?([^:]*?\n)?(\d\d:\d\d)/);
        const stations = stationColumn.substr(idxStations).match(/(.*)\n[ ]+(.*)(?:\n|,\n  +(.*)\n)(?:.*\n(?:.*\n)*)?.*(?:Bus|Autobus|Zug) +(.*)\n.*(?:Direction|à destination de|Kierunek|richting|Richtung) (.*)\n(.*)\n(?:[ ]+(.*?)(?:\n|,\n +(.*)\n))?/);
        if (!times || !stations) {
            break;
        }
        idxTime += times.index + times[0].length;
        idxStations += stations.index + stations[0].length;

        let res = JsonLd.newBusReservation();
        res.reservationNumber = resNum;

        res.reservedTicket.ticketToken = 'qrCode:' + triggerNode.content;
        res.reservationFor.departureTime = parseDate(date[2], date[1], times[3] ? times[2] : null, times[1]);
        res.reservationFor.departureBusStop.name = stations[1];
        parseLocation(res.reservationFor.departureBusStop, stations[2], stations[3], links);

        res.reservationFor.busNumber = stations[4].match(/(.*?)(?:  |$)/)[1];
        res.reservationFor.busName = stations[5];

        res.reservationFor.arrivalTime = parseDate(date[2], date[1], times[3] ? times[3] : times[2], times[4]);
        res.reservationFor.arrivalBusStop.name = stations[6];
        parseLocation(res.reservationFor.arrivalBusStop, stations[7], stations[8], links);

        reservations.push(fixTransportMode(res));
    }

    if (reservations.length > 1) // unclear how to match seats in that case
        return reservations;

    const rightSide = page.textInRect(0.47, 0.0, 1.0, 0.5);
    let idx = 0;
    let personalizedReservations = [];
    while (true) {
        const pas = rightSide.substr(idx).match(/(\S+.*\S)  +(?:(\d+) · )?(\d+[A-Z])\n/);
        if (!pas) {
            break;
        }
        idx += pas.index + pas[0].length;
        for (res of reservations) {
            let r = JsonLd.clone(res);
            r.underName.name = pas[1];
            r.reservedTicket.ticketedSeat.seatNumber = pas[3];
            r.reservedTicket.ticketedSeat.seatSection = pas[2];
            personalizedReservations.push(r);
        }
    }
    return personalizedReservations.length ? personalizedReservations : reservations;
}
