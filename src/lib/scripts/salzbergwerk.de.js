// SPDX-FileCopyrightText: 2024 David Pilarčík <meow@charliecat.space>
// SPDX-License-Identifier: LGPL-2.0-or-later

function main(pdf, node, barcode) {

    const text = pdf.text
    const res = JsonLd.newEventReservation();

    let reservationNumber = text.match(/[0-9]{12}/gm)[0]
    let tourStart         = text.match(/([0-9]+(\.[0-9]+)+) at (0?[0-9]|1[0-9]|2[0-3]):[0-9]{2}/gm)[0]
    let tourName          = text.match(/This is your ticket for (.*?) on the/)[1]
    /*let arrivalAddress    = text.match(/ARRIVAL\S+ROUTE PLANNER\S+(.*)\S+More detailed arrival\S+(.*)\S+information you will find under\S+D - ([0-9]+)\S(.*)\S+www\.salzbergwerk\.de/gm)*/

    res.reservationFor.startDate = JsonLd.toDateTime(tourStart.replace("at ", ""), "dd.MM.yyyy hh:mm", "de")
    res.reservationFor.name = tourName

    res.reservationFor.location.name = "Parking Salzbergwerk";
    res.reservationFor.location.address.streetAddress = "Salzburger Straße 24";
    res.reservationFor.location.address.postalCode = "83471";
    res.reservationFor.location.address.addressLocality = "Berchtesgaden";
    res.reservationFor.location.address.addressCountry = "DE";

    res.reservedTicket.ticketToken = "datamatrix:" + reservationNumber
    res.reservationNumber = reservationNumber

    return res

}
