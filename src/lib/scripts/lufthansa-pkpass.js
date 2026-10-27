/*
   SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

function main(pass, node)
{
    let res = node.result[0];
    if (res["@type"] === "FlightReservation") {
        res.reservationFor.departureAirport.name = pass.field["origin"].label;
        res.reservationFor.arrivalAirport.name = pass.field["destination"].label;
        if (pass.field["group"])
            res.boardingGroup = pass.field["group"].value;
    } else {
        res.reservationFor.departureStation.name = pass.field["origin"].label;
        res.reservationFor.departureStation.identifier = "iata:" + pass.field["origin"].value;
        res.reservationFor.arrivalStation.name = pass.field["destination"].label;
        res.reservationFor.arrivalStation.identifier = "iata:" + pass.field["destination"].value;
        res.reservationFor.arrivalTime = JsonLd.toDateTime(pass.field["arrivalTime"]?.value, "HH:mm", "de");
        res.reservationFor.trainNumber = pass.field["trainName"]?.value;
    }

    const back = pass.backFields;
    const cancelLink = back.find(item => item.key === "cancel");
    if (cancelLink) {
        let cancelUrl = cancelLink.value.match(/https:\/\/mobile.lufthansa.com\/service\/checkin\?[A-Z0-9=&]*[A-Z0-9=&]/i);
        if (!cancelUrl) {
            cancelUrl = cancelLink.value.match(/https:\/\/once.lufthansa.com\/[^"]*/i);
        }
        if (cancelUrl) {
            res.potentialAction = JsonLd.newObject("CancelAction");
            res.potentialAction.url = cancelUrl[0];
        }
    }

    return res;
}
