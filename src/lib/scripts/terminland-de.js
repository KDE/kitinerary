/*
   SPDX-FileCopyrightText: 2020 Nicolas Fella <nicolas.fella@gmx.de>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

function main(doc) {

    var res = JsonLd.newEventReservation();

    const dateString = doc.eval("//div/div/span")[0].content.replace("\n", " ").match(".*, (.*) Uhr")[1]

    const addressBlock = doc.eval("//div/div")[2].content.split("\n")

    const placeName = addressBlock[2]

    const street = addressBlock[4]

    const zipAndCity = addressBlock[5]

    const zip = zipAndCity.match(/([0-9]*)/)[1]

    const city = zipAndCity.match(/[0-9]* (.*)/)[1]

    const firstName = doc.eval("//div/div/table/tr/td")[2].content

    const lastName = doc.eval("//div/div/table/tr/td")[5].content

    const cancelUrl = doc.eval("//div/div/div/a")[1].content

    const address = JsonLd.newObject("PostalAddress")
    address.addressCountry = "DE"
    address.addressLocality = city
    address.postalCode = zip
    address.streetAddress = street

    res.reservationFor.location.address = address
    res.reservationFor.location.name = placeName

    res.reservationFor.startDate = JsonLd.toDateTime(dateString, "dd.MM.yyyy hh:mm", "de")

    res.reservationFor.name = firstName + " " + lastName

    res.potentialAction = JsonLd.newObject("CancelAction")
    res.potentialAction.url = cancelUrl

    return res
}
