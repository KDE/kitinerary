// SPDX-FileCopyrightText: 2025 Volker Krause <vkrause@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

function extractPass(pass, node) {
    let res = node.result[0];
    res.reservationFor.startDate = JsonLd.toDateTime(pass.field['dateTime'].value, 'yyyy.MM.dd. HH:mm', 'hu');
    res.reservationFor.location.name = 'Müpa';
    return res;
}
