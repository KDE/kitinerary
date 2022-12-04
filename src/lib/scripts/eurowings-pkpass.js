/*
   SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

function main(pass, node)
{
    var res = node.result[0];
    res.reservationFor.departureAirport.name = pass.field["origin"].label;
    res.reservationFor.arrivalAirport.name = pass.field["destination"].label;
    if (pass.field["operatingcarrier"])
        res.reservationFor.airline.name = pass.field["operatingcarrier"].value;

    const secondary = pass.secondaryFields;
    const boardingGroup = secondary.find(item => item.key === "boardinggroup");
    if (boardingGroup) {
        res.boardingGroup = boardingGroup.value;
    }

    return res;
}
