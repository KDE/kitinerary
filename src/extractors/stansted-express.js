/*
   Copyright (c) 2019 Volker Krause <vkrause@kde.org>

   This library is free software; you can redistribute it and/or modify it
   under the terms of the GNU Library General Public License as published by
   the Free Software Foundation; either version 2 of the License, or (at your
   option) any later version.

   This library is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
   FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
   License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to the
   Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
   02110-1301, USA.
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

function parsePkPass(pass)
{
    var res = JsonLd.newObject("TrainReservation");
    res.reservationFor = JsonLd.newObject("TrainTrip");
    res.reservationFor.departureDay = JsonLd.toDateTime(pass.field["travelDate"].value, "dd/MM/yyyy", "en");
    res.reservationFor.departureStation = JsonLd.newObject("TrainStation");
    res.reservationFor.departureStation.name = expandStationName(pass.field["from"].value);
    res.reservationFor.arrivalStation = JsonLd.newObject("TrainStation");
    res.reservationFor.arrivalStation.name = expandStationName(pass.field["to"].value);

    return res;
}
