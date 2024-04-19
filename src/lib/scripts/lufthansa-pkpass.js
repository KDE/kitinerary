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
    } else {
        res.reservationFor.departureStation.name = pass.field["origin"].label;
        res.reservationFor.arrivalStation.name = pass.field["destination"].label;
    }

    const back = pass.backFields;
    const cancelLink = back.find(item => item.key === "cancel");
    if (cancelLink) {
        const cancelUrl = cancelLink.value.match(/https:\/\/mobile.lufthansa.com\/service\/checkin\?[A-Z0-9=&]*[A-Z0-9=&]/i);
        if (cancelUrl) {
            res.potentialAction = JsonLd.newObject("CancelAction");
            res.potentialAction.url = cancelUrl[1];
        }
    }

    return res;
}
