/*
   SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
   SPDX-License-Identifier: LGPL-2.0-or-later
*/

function parsePass(content, node) {
    var res = node.result[0];
    res.reservationFor.name = content.field['eventName'].value;
    res.reservationFor.startDate = JsonLd.toDateTime(content.field['doorsOpen'].value, 'dd.MM.yyyy hh:mm', 'de');
    res.reservationFor.endDate = JsonLd.toDateTime(content.field['doorsClose'].value, 'dd.MM.yyyy hh:mm', 'de');
    res.reservationFor.url = content.field['website'].value;
    res.underName = JsonLd.newObject('Person');
    res.underName.name = content.field['name'].value;
    res.reservationNumber = content.field['orderCode'].value;
    return res;
}
