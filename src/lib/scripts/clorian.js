// SPDX-FileCopyrightText: 2026 David Pilarčík <meow@charliecat.space>
// SPDX-License-Identifier: LGPL-2.0-or-later

function parsePkPass(pass, node) {
    const res = node.result[0];

    ExtractorEngine.extractPrice(pass.field["amount"]?.value ?? "0 ", res)
    res.reservedTicket.name = `${pass.field["product"].value} - ${pass.field["buyer_type"].value}`
    res.reservationFor.url = pass.field["client_url"]?.value
    res.reservationNumber = pass.field["loc_reservation"]?.value
    res.reservedTicket.ticketNumber = pass.field["loc_ticket"]?.value
    res.reservationFor.location.name = pass.field["venue_access"]?.value

    return res
}

function parsePdfTicket(pdf, node, barcode) {
    const res = JsonLd.newEventReservation()
    const page = pdf.pages[barcode.location];

    // Sample might be venue specific template

    res.reservedTicket.ticketToken = 'qrcode:' + barcode.content; // QR code is generated in PkPasses, sample has datamatrix

    const ticketTypeGeneral = page.textInRect(0, 0.04, 0.3, 0.1)
    const ticketTypeGroup = page.textInRect(0, 0.12, 0.3, 0.2)
    res.reservedTicket.name = `${ticketTypeGeneral} - ${ticketTypeGroup}`

    // ["07/08/2026", "11:00 - 11:45"] // may be sample venue's specific field
    let [date, timeStartEnd] = page.textInRect(0.2, 0.1, 0.5, 0.2).split("\n")
    let [timeStart, timeEnd] = timeStartEnd.split(" - ")
    res.reservationFor.startDate = JsonLd.toDateTime(`${date} ${timeStart}`, ["dd/MM/yyyy hh:mm"], ["en", "es"])
    res.reservationFor.endDate = JsonLd.toDateTime(`${date} ${timeEnd}`, ["dd/MM/yyyy hh:mm"], ["en", "es"])

    // R: 104203278\nT: 309736409
    // [_, "104203278", "309736409"]
    let [_, reservationNumber, ticketNumber] = /R: (\d+)\s+T: (\d+)/gm.exec(page.textInRect(0.5, 0.1, 0.7, 0.2))
    res.reservationNumber = reservationNumber
    res.reservedTicket.ticketNumber = ticketNumber

    res.reservationFor.name = "Event"

    return res
}
