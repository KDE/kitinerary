/*
   SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

function parsePdf(pdf, node, triggerNode)
{
    var obj = JSON.parse(triggerNode.content);
    if (!obj)
        return null;

    var res = JsonLd.newTrainReservation();
    res.underName.givenName = obj.name;
    res.underName.familyName = obj.surname
    res.reservationNumber = obj.idDocValue;
    res.reservationFor.trainNumber = obj.nrKursu;
    res.reservationFor.departureStation.name = obj.fromStop;
    res.reservationFor.arrivalStation.name = obj.toStop;
    var depDate = new Date();
    depDate.setTime(obj.goDate);
    res.reservationFor.departureTime = depDate;

    return res;
}
