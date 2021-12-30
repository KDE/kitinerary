/*
    SPDX-FileCopyrightText: 2021 Nicolas Fella <nicolas.fella@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

function main(content) {
    var res = JsonLd.newEventReservation();

    var name = content.match(/Name: (.*)/)[1]
    var theDate = content.match(/Termin Datum: (.*)/)[1]
    var theTime = content.match(/Termin Uhrzeit: (.*) Uhr/)[1]
    var addr = content.match(/Adresse der Teststation: (.*), (.*), ([0-9]*) (.*)/)

    var cancelUrlMatch = content.match(/Für Absagen verwenden Sie bitte folgenden Link: (.*)/)

    const address = JsonLd.newObject("PostalAddress")
    address.addressCountry = "DE"
    address.addressLocality = addr[4]
    address.postalCode = addr[3]
    address.streetAddress = addr[2]

    res.reservationFor.name = name

    // FIXME some cancel urls are not matched
    if (cancelUrlMatch) {
        res.potentialAction = JsonLd.newObject("CancelAction")
        res.potentialAction.url = cancelUrlMatch[1]
    }

    res.reservationFor.location.address = address
    res.reservationFor.location.name = addr[1]

    // Both hh::mm and hh.mm are observed in the wild
    var dateTime = JsonLd.toDateTime(theDate + " " + theTime, "dd.MM.yyyy hh:mm", "de")

    if (isNaN(dateTime.getTime())) {
        dateTime = JsonLd.toDateTime(theDate + " " + theTime, "dd.MM.yyyy hh.mm", "de")
    }

    res.reservationFor.startDate = dateTime

    return res
}
