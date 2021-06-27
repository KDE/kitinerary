/*
   SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
   SPDX-License-Identifier: LGPL-2.0-or-later
*/

function parseReservation(pass, node) {
    var res = node.result[0];
    res.reservationFor.name = pass.field['valid-locations'].value;
    res.reservationFor.startDate = pass.field['valid-date'].value;
    const endTime = pass.field['valid-time-range'].value.match(/-(\d+):(\d+)/);
    var endDt = pass.field['valid-date'].value;
    endDt.setHours(endTime[1]);
    endDt.setMinutes(endTime[2]);
    res.reservationFor.endDate = endDt;
    res.reservationFor.url = pass.field['website'].value;
    res.underName = JsonLd.newObject("Person");
    res.underName.name = pass.field['customer_name'].value;
    return res;
}
