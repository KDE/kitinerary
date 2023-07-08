/*
   SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
   SPDX-License-Identifier: LGPL-2.0-or-later
*/

function extractPass(pass, node) {
    let res = node.result[0];
    res.reservationFor = JsonLd.newObject("BusTrip");
    res.reservationFor.departureTime =  JsonLd.toDateTime(pass.field["date"].value + ' ' + pass.field["origin"].value, "dd.MMM hh:mm", "en");
    res.reservationFor.departureBusStop = {
        '@type': 'BusStation',
        name: pass.field["origin_long"].value,
        address: {
            '@type': 'PostalAddress',
            addressLocality: pass.field["origin"].label
        },
        geo: {
            '@type': 'GeoCoordinates',
            latitude: pass.locations[0].latitude,
            longitude: pass.locations[0].longitude
        }
    }
    res.reservationFor.arrivalTime =  JsonLd.toDateTime(pass.field["date"].value + ' ' + pass.field["destination"].value, "dd.MMM hh:mm", "en");
    res.reservationFor.arrivalBusStop = {
        '@type': 'BusStation',
        name: pass.field["destination_long"].value,
        address: {
            '@type': 'PostalAddress',
            addressLocality: pass.field["destination"].label
        }
    };
    res.reservedTicket.ticketedSeat = {
        '@type': 'Seat',
        seatNumber: pass.field["tariff"].value.match(/\/(.*)/)[1]
    };
    return res;
}
