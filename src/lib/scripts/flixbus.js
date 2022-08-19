/*
   SPDX-FileCopyrightText: 2019-2022 Volker Krause <vkrause@kde.org>
   SPDX-License-Identifier: LGPL-2.0-or-later
*/

function main(content, node) {
    // convert QR download links into the actual QR codes
    var res = node.result;
    for (var i = 0; i < res.length; ++i) {
        var ticketToken = res[i].reservedTicket.ticketToken;
        res[i].reservedTicket.ticketToken = ticketToken.replace(/^https?:\/\/api\.(?:flixbus|meinfernbus)\..{2,3}\/qrcode\/(..)\//, "qrCode:https://shop.flixbus.$1/pdfqr/");

        // their schema.org annotations also claim train trips are bus trips, fix that
        if (res[i].reservationFor.departureBusStop.name.endsWith(" (FlixTrain)") && res[i].reservationFor.arrivalBusStop.name.endsWith(" (FlixTrain)")) {
            res[i] = JsonLd.busToTrainReservation(res[i]);
            res[i].reservationFor.departureStation.name = res[i].reservationFor.departureStation.name.substr(0, res[i].reservationFor.departureStation.name.length - 11);
            res[i].reservationFor.arrivalStation.name = res[i].reservationFor.arrivalStation.name.substr(0, res[i].reservationFor.arrivalStation.name.length - 11);
        }
    }
    return res;
}

function parseDate(year, baseDate, overrideDate, time)
{
    const s = (overrideDate ? overrideDate.trim() : baseDate) + ' ' + year + ' ' + time;
    return JsonLd.toDateTime(s, 'd MMM yyyy hh:mm', ['en', 'fr']);
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
    const date = text.match(/^\S+,? (\d+ \S+) (\d{4})\n/);
    let idx = date.index + date[0].length;
    let reservations = [];
    while (true) {
        const dep = text.substr(idx).match(/(\d\d:\d\d)  +(.*)\n(\d{1,2} \S+)?(?:  + (.*?)(?:\n|,\n  +(.*)\n))?/);
        if (!dep) break;
        idx += dep.index + dep[0].length;
        const bus = text.substr(idx).match(/^[ ]+ Bus +(.*)\n[ ]+(?:Direction|à destination de) (.*)\n/);
        if (!bus) break;
        idx += bus.index + bus[0].length;
        const arr = text.substr(idx).match(/^(\d{1,2} \S+\n)?(\d\d:\d\d)  +(.*)\n(?:  + (.*?)(?:\n|,\n  +(.*)\n))?/);
        if (!arr) break;
        idx += arr.index + arr[0].length;

        let res = JsonLd.newBusReservation();
        res.reservationNumber = resNum;

        res.reservedTicket.ticketToken = 'qrCode:' + triggerNode.content;
        res.reservationFor.departureTime = parseDate(date[2], date[1], dep[3], dep[1]);
        res.reservationFor.departureBusStop.name = dep[2];
        parseLocation(res.reservationFor.departureBusStop, dep[4], dep[5], links);

        res.reservationFor.busNumber = bus[1];
        res.reservationFor.busName = bus[2];

        res.reservationFor.arrivalTime = parseDate(date[2], date[1], arr[1], arr[2]);
        res.reservationFor.arrivalBusStop.name = arr[3];
        parseLocation(res.reservationFor.arrivalBusStop, arr[4], arr[5], links);

        reservations.push(res);
    }
    return reservations;
}
