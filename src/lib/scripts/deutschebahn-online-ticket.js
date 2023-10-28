/*
   SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
   SPDX-License-Identifier: LGPL-2.0-or-later
*/

function parseLocation(node, location)
{
    location.name = node.eval('n')[0].content;
    location.identifier = 'ibnr:' + node.eval('nr')[0].content;
    const plz = node.eval('plz');
    if (plz.length > 0)
        location.address.postalCode = plz[0].content;
    const x = node.eval('x');
    if (x.length > 0)
        location.geo.longitude = x[0].content / 1000000;
    const y = node.eval('y');
    if (y.length > 0)
        location.geo.latitude = y[0].content / 1000000;
}

function parseOnlineTicket(xml)
{
    // TODO handle multi-ticket

    const trains = xml.root.eval('//trainlist/train');
    let result = [];
    for (const train of trains) {
        let res = JsonLd.newTrainReservation();
        res.reservationFor.trainNumber = train.attribute('tn');
        const dep = train.eval('dep')[0];
        parseLocation(dep, res.reservationFor.departureStation);
        const depPtfMatch = dep.eval('ptf');
        if (depPtfMatch.length > 0) {
            res.reservationFor.departurePlatform = depPtfMatch[0].content;
        }
        res.reservationFor.departureTime = dep.attribute('dt').substr(0, 11) + dep.attribute('t');

        const arr = train.eval('arr')[0];
        parseLocation(arr, res.reservationFor.arrivalStation);
        const arrPtfMatch = arr.eval('ptf');
        if (arrPtfMatch.length > 0) {
            res.reservationFor.arrivalPlatform = arrPtfMatch[0].content;
        }
        res.reservationFor.arrivalTime = arr.attribute('dt').substr(0, 11) + arr.attribute('t');

        if (train.eval('gat')[0].content === "Bus") {
            res = JsonLd.trainToBusReservation(res);
        }

        result.push(res);
    }

    const reslist = xml.root.eval('//reslist/res');
    for (const seat of reslist) {
        const tn = seat.attribute('tn');
        for (const train of result) {
            if (train.reservationFor.trainNumber != tn) {
                continue;
            }
            // TODO maybe better to use plaetze/platz/platznr|wagennr, but needs a multi-seat example
            train.reservedTicket.ticketedSeat.seatSection = seat.eval('nvplist/nvp[@name="wagennummer"]')[0].content;
            const plaetze2 = seat.eval('nvplist/nvp[@name="plaetze2"]');
            if (plaetze2.length > 0)
                train.reservedTicket.ticketedSeat.seatNumber = plaetze2[0].content;
            else {
                train.reservedTicket.ticketedSeat.seatNumber = seat.eval('txt')[0].content.match(/Pl. (\d+)/)[1];
            }
            break;
        }
    }

    const tickets = xml.root.eval('//tcklist/tck');
    if (tickets.length != 1) {
        console.warn("Multi-ticket support missing!");
        return result;
    }

    const barcode = tickets[0].eval('//htdata/ht[@name="barcode"]');
    let ticket = {};
    if (barcode.length > 0) {
        ticket = ExtractorEngine.extract(ByteArray.fromBase64(barcode[0].content.substr(22).trim())).result[0];
        ticket.reservedTicket.ticketNumber = tickets[0].eval('mtk/ot_nr_hin')[0].content;
    } else {
        const barcode = tickets[0].eval('//htdata/ht')[0];
        ticket = ExtractorEngine.extract(ByteArray.fromBase64(barcode.content.substr(0).trim())).result[0];
    }
    let mergedResult = [];
    for (let train of result) {
        if (train['@type'] == 'BusReservation') {
            mergedResult.push(JsonLd.apply(JsonLd.trainToBusReservation(ticket), train));
        } else {
            mergedResult.push(JsonLd.apply(ticket, train));
        }
    }
    return mergedResult;
}
