/*
    SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

function parsePass(pass, node) {
    let res = node.result[0];
    res.reservationFor.name = pass.field['eventtitle'].value;
    const addr = pass.field['address'].value.split('\n');
    res.reservationFor.location.name = addr[0];
    res.reservationFor.location.address = {
        '@type': 'PostalAddress',
        streetAddress: addr[1],
        addressLocality: addr[2]
    };
    return res;
}
