/*
   SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
   SPDX-License-Identifier: LGPL-2.0-or-later
*/

function extractPass(pass, node) {
    let res = node.result[0];
    console.log(pass);
    res.reservedTicket.name = pass.field['event'].label;
    const times = pass.field['event'].value.match(/(\d\d:\d\d)-(\d\d:\d\d)/);
    res.reservationFor.startDate = res.reservationFor.startDate.substr(0, 11) + times[1];
    res.reservationFor.endDate = res.reservationFor.startDate.substr(0, 11) + times[2];
    const loc = pass.field['loc'].value.split('\n');
    res.reservationFor.location.name = loc[0];
    res.reservationFor.location.address = {
        streetAddress: loc[1],
        addressLocality: loc[2]
    };
    res.underName = {
        '@type': 'Person',
        name: pass.field['back_ticketOwner'].value
    };
    return res;
}
