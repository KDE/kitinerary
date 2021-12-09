/*
   SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
   SPDX-License-Identifier: LGPL-2.0-or-later
*/

function parseTicket(pdf, node, triggerNode) {
    const page = pdf.pages[triggerNode.location];
    const text = page.text;

    var legs = new Array();
    var idx = 0;
    while (true) {
        const train = text.substr(idx).match(/TRAIN  +\w.*?  +(.*?) +DEPARTS\s+ARRIVES \(\w{3} (.*)\)\n\s*(\d+)\s+(\w{3} \d{1,2}), (\d{4})\n\s*(.*)?\n?\s*\d+ (.*) Seats?\n?  +(\d{1,2}:\d{2} [AP]M) +(\d{1,2}:\d{2} [AP]M)/);
        if (!train) {
            break;
        }
        idx += train.index + train[0].length;

        var leg = JsonLd.newTrainReservation();
        leg.reservedTicket.ticketToken = 'qrcode:' + triggerNode.content;
        leg.reservationFor.trainNumber = train[3];
        leg.reservationFor.departureTime = JsonLd.toDateTime(train[4] + ' ' + train[5] + ' ' + train[8], 'MMM d yyyy h:mm AP', 'en');
        leg.reservationFor.arrivalTime = JsonLd.toDateTime(train[2] + ' ' + train[5] + ' ' + train[9], 'MMM d yyyy h:mm AP', 'en');
        leg.reservedTicket.ticketedSeat.seatingType = train[7];

        const stations = (train[1] + ' ' + (train[6] ?? '')).match(/(.*) - (.*)/);
        leg.reservationFor.departureStation.name = stations[1];
        leg.reservationFor.arrivalStation.name = stations[2];
        legs.push(leg);
    }

    var reservations = new Array();
    const pasHdr = text.match(/PASSENGERS \((\d+)\).*\n/);
    const pasCount = pasHdr[1];
    idx = pasHdr.index + pasHdr[0].length;
    for (var i = 0; i < pasCount; ++i) {
        const pas = text.substr(idx).match(/([^,]*), (.*?)  .*?(\d{10})?(?: \| .*)?\n/);
        idx += pas.index + pas[0].length;
        for (const leg of legs) {
            var res = JsonLd.clone(leg);
            res.underName.givenName = pas[2];
            res.underName.familyName = pas[1];
            if (pas[3]) {
                res.programMembershipUsed.membershipNumber = pas[3];
            }
            reservations.push(res);
        }
    }

    return reservations;
}
