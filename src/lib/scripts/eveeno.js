// SPDX-FileCopyrightText: © 2026 Volker Krause <vkrause@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

function extractPass(pass, node) {
    let res = node.result[0];
    res.reservationFor.location.name = pass.field["location"].value;
    res.reservationNumber = pass.field["ticketnr"].value;
    res.underName = {
        '@type': 'Person',
        name: pass.field["ticketowner"].value
    };
    return res;
}
