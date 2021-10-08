/*
   SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

function main(pass, node)
{
//     if (pass.transitType != KPkPass.BoardinPass.Air) { // TODO this needs to be registered in the engine
//         return null;
//     }

    var res = node.result[0];
    res.reservationFor.departureAirport.name = pass.field["origin"].label;
    res.reservationFor.arrivalAirport.name = pass.field["destination"].label;

    const secondary = pass.secondaryFields;
    const passengerName = secondary.find(item => item.key === "passenger");
    if (passengerName) {
        res.underName = JsonLd.newObject("Person");
        res.underName.name = passengerName.value;
    }

    return res;
}
