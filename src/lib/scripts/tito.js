// SPDX-FileCopyrightText: 2024 Volker Krause <vkrause@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

function extractPass(pass, node) {
    let res = node.result[0];
    res.reservationFor.name = pass.logoText;
    res.reservedTicket.name = pass.field['ticket'].value;
    res.reservationFor.location = {
        '@type': 'Place',
        name: pass.field['location'].value
    };
    if (pass.field['reference'])
        res.reservationNumber = pass.field['reference'].value;
    res.underName = {
        '@type': 'Person',
        name: pass.secondaryFields.find(item => item.key === 'name').value,
        email: pass.field['email'] ? pass.field['email'].value : null
    }
    res.provider = {
        '@type': 'Organization',
        email: pass.field['contact_email'] ? pass.field['contact_email'].value : null
    };

    const date = pass.field['date'].value;
    if (dt = date.match(/(\S+ \d{1,2})\S{2}, (\d{4})/)) {
        res.reservationFor.startDate = JsonLd.toDateTime(dt[1] + dt[2], "MMMM ddyyyy", "en");
    } else if (dt = date.match(/(\S+) (\d{1,2})\S{2}\S(\d{1,2})\S{2}, (\d{4})/)) {
        console.log(dt);
        res.reservationFor.startDate = JsonLd.toDateTime(dt[1] + ' ' + dt[2] + dt[4], "MMMM ddyyyy", "en");
        res.reservationFor.endDate = JsonLd.toDateTime(dt[1] + ' ' + dt[3] + dt[4], "MMMM ddyyyy", "en");
    }


    return res;
}
