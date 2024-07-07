/*
    SPDX-FileCopyrightText: 2024 David Pilarćík <meow@charliecat.space>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

function parsePkPass(pass, node) {
    const res = node.result[0];

    res.totalPrice = pass.field["price"].value
    res.priceCurrency = pass.field["price"].currencyCode

    const address = res.reservationFor.location.address = JsonLd.newObject("PostalAddress")
    address.streetAddress = pass.field['venue address'].value

    return res
}

function parsePdfTicket(pdf, node, triggerNode) {
    const result = []

    for (let page of pdf.pages) {
        const res = JsonLd.newEventReservation()

        res.reservedTicket.ticketToken = 'qrCode:' + (res.reservationNumber = page.textInRect(0.7, 0.1, 1, 0.2))
        res.reservationFor.name = page.textInRect(0.0, 0.05, 1, 0.11)
        res.reservationFor.startDate = JsonLd.toDateTime(page.textInRect(0, 0.1, 0.3, 0.13).replace(/^\w{3}\s/, ''), 'd/M/yyyy HH:mm', 'en') // Sat 14/9/2024 14:00

        // NOTE: Not the best representation, GoOut PDF sample ticket shows name of a city quarter than the actual city
        //		 therefor cant be certain with this information. This is only taking the Venue Name
        res.reservationFor.location.name = page.textInRect(0.3, 0.1, 0.9, 0.13)

        result.push(res)
    }

    return result
}
