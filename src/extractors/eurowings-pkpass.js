/*
   SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

function main(pass)
{
    var res = Context.data[0];
    res.reservationFor.departureAirport.name = pass.field["origin"].label;
    res.reservationFor.arrivalAirport.name = pass.field["destination"].label;
    if (pass.field["operatingcarrier"])
        res.reservationFor.airline.name = pass.field["operatingcarrier"].value;

    return res;
}
