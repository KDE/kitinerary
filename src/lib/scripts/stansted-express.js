/*
   SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

function expandStationName(name)
{
    switch (name) {
        case "SSD": return "Stansted";
        case "LST": return "London Liverpool Street";
        case "TOM": return "Tottenham Hale";
        case "SRA": return "Stratford";
    }
    return name;
}

function parsePkPass(pass, node)
{
    var res = node.result[0];
    res.reservationFor = JsonLd.newObject("TrainTrip");
    res.reservationFor.departureDay = JsonLd.toDateTime(pass.field["travelDate"].value, "dd/MM/yyyy", "en");
    res.reservationFor.departureStation = JsonLd.newObject("TrainStation");
    res.reservationFor.departureStation.name = expandStationName(pass.field["from"].value);
    res.reservationFor.departureStation.identifier = 'uk:' + pass.field["from"].value;
    res.reservationFor.arrivalStation = JsonLd.newObject("TrainStation");
    res.reservationFor.arrivalStation.name = expandStationName(pass.field["to"].value);
    res.reservationFor.arrivalStation.identifier = 'uk:' + pass.field["to"].value;

    return res;
}
