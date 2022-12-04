/*
   SPDX-FileCopyrightText: 2022 Kai Uwe Broulik <kde@broulik.de>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

function main(pass, node)
{
    let res = node.result[0];

    const departureTime = JsonLd.toDateTime(pass.field["departuretime"].value, "hh:mm", "en");
    if (!isNaN(departureTime.getTime())) {
        res.reservationFor.departureTime = departureTime;
    }

    const arrivalTime = JsonLd.toDateTime(pass.field["arrivaltime"].value, "hh:mm", "en");
    if (!isNaN(arrivalTime.getTime())) {
        res.reservationFor.arrivalTime = arrivalTime;
    }

    return res;
}
