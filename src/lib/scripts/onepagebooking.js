/*
   SPDX-FileCopyrightText: 2021 Nicolas Fella <nicolas.fella@gmx.de>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

function main(content) {

    const resId = content.match(/Buchungsnummer:\n?\s*(.+)/)[1]

    const arrivalDate = content.match(/Anreisetag:\n? (.+)/)[1]
    const departureDate = content.match(/Abreisetag:\n? (.+)/)[1]

    const guestName = content.match(/Gastname:\n? (?:Herr|Frau)? ?(.+)/)[1]

    const addressBlock = content.match(/Ihr\n(.*)\n(.*)\n(.*)/)

    const hotelName = addressBlock[1]

    const street = addressBlock[2]

    const cityLine = addressBlock[3].match(/([0-9]+) (.*)/)

    const address = JsonLd.newObject("PostalAddress")
    address.addressCountry = "DE"
    address.addressLocality = cityLine[2]
    address.postalCode = cityLine[1]
    address.streetAddress = street

    const telephone = content.match(/Tel.: (.*)/)[1]
    const email = content.match(/E-Mail:  (.*)/)[1]

    const price = content.match(/Gesamtpreis:\n? (.*) EUR/)[1].replace(',', '.')

    const numberAdults = content.match(/Anzahl der Erwachsene[rn]:\n? ([0-9]+)/)[1]

    const numberChildren = content.match(/Anzahl der Kinder:\n? ([0-9]+)/)[1]

    var res = JsonLd.newLodgingReservation()

    res.reservationFor.name = addressBlock[1]

    res.reservationNumber = resId
    res.checkinTime = JsonLd.toDateTime(arrivalDate, "dd.MM.yyyy", "de")
    res.checkoutTime = JsonLd.toDateTime(departureDate, "dd.MM.yyyy", "de")

    res.reservationFor.telephone = telephone
    res.reservationFor.email = email

    res.reservationFor.address = address

    res.underName.name = guestName

    res.totalPrice = price
    res.priceCurrency = "EUR"

    res.reservationFor.numAdults = numberAdults
    res.reservationFor.numChildren = numberChildren

    return res
}
