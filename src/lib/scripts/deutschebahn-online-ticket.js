/*
   SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
   SPDX-License-Identifier: LGPL-2.0-or-later
*/

function parseOnlineTicket(xml)
{
    // TODO handle multi-ticket
    // TODO how do buses look in here?

    const trains = xml.root.eval('//trainlist/train');
    let result = [];
    for (const train of trains) {
        let res = JsonLd.newTrainReservation();
        res.reservationFor.trainNumber = train.attribute('tn');
        const dep = train.eval('dep')[0];
        res.reservationFor.departureStation.name = dep.eval('n')[0].content;
        res.reservationFor.departureStation.identifier = 'ibnr:' + dep.eval('nr')[0].content;
        res.reservationFor.departureStation.address.postalCode = dep.eval('plz')[0].content;
        res.reservationFor.departureStation.geo.longitude = dep.eval('x')[0].content / 1000000;
        res.reservationFor.departureStation.geo.latitude = dep.eval('y')[0].content / 1000000;
        res.reservationFor.departurePlatform = dep.eval('ptf')[0].content;
        res.reservationFor.departureTime = dep.attribute('dt').substr(0, 11) + dep.attribute('t');

        const arr = train.eval('arr')[0];
        res.reservationFor.arrivalStation.name = arr.eval('n')[0].content;
        res.reservationFor.arrivalStation.identifier = 'ibnr:' + arr.eval('nr')[0].content;
        res.reservationFor.arrivalStation.address.postalCode = arr.eval('plz')[0].content;
        res.reservationFor.arrivalStation.geo.longitude = arr.eval('x')[0].content / 1000000;
        res.reservationFor.arrivalStation.geo.latitude = arr.eval('y')[0].content / 1000000;
        res.reservationFor.arrivalPlatform = arr.eval('ptf')[0].content;
        res.reservationFor.arrivalTime = arr.attribute('dt').substr(0, 11) + arr.attribute('t');
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
            train.reservedTicket.ticketedSeat.seatNumber = seat.eval('nvplist/nvp[@name="plaetze2"]')[0].content;
            break;
        }
    }

    const tickets = xml.root.eval('//tcklist/tck');
    if (tickets.length != 1) {
        console.warn("Multi-ticket support missing!");
        return result;
    }

    const barcode = tickets[0].eval('//htdata/ht[@name="barcode"]')[0];
    const ticket = ExtractorEngine.extract(ByteArray.fromBase64(barcode.content.substr(22).trim())).result[0];
    ticket.reservedTicket.ticketNumber = tickets[0].eval('mtk/ot_nr_hin')[0].content;
    let mergedResult = [];
    for (let train of result) {
        mergedResult.push(JsonLd.apply(ticket, train));
    }
    return mergedResult;
}
