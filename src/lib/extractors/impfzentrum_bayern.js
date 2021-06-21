/*
    SPDX-FileCopyrightText: 2021 Nicolas Fella <nicolas.fella@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

function main(content) {
    var res = JsonLd.newEventReservation();

    var splitted = content.split("\n")

    var name = splitted[0].slice(0, -1)
    res.reservationFor.name = name

    var firstShot = splitted[4].match(/Erste Impfung: (.*) Uhr/)

    var secondShot = splitted[4].match(/Zweite Impfung: (.*) Uhr/)

    var theDate

    if (firstShot) {
        theDate = firstShot[1]
    } else {
        theDate = secondShot[1]
    }

    const address = JsonLd.newObject("PostalAddress")
    address.addressCountry = "DE"
    address.addressLocality = splitted[11].split(" ")[1]
    address.addressRegion = "Bayern"
    address.postalCode = splitted[11].split(" ")[0]
    address.streetAddress = splitted[10]

    res.reservationFor.location.address = address

    res.reservationFor.startDate = JsonLd.toDateTime(theDate, "dd.MM.yyyy hh:mm", "de")

    return res
}
