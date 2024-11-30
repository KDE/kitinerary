/*
    SPDX-FileCopyrightText: 2024 David Pilarćík <meow@charliecat.space>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

function parsePkPass(pass, node) {
    const res = node.result[0];

    let _price = (pass.field["price"]?.value ?? "0 ").split(" ")
    res.totalPrice = Number(_price[0])
    res.priceCurrency = _price[1]

    res.reservationFor.name = pass.field["eventname"].value
    res.reservedTicket.name = pass.field["section"].value

    return res
}

function parsePdf(pdf, node, triggerNode) {
    const results = []

    for (let i = 0; i < pdf.pages.length; i++) {
        const page = pdf.pages[i]
        let res = JsonLd.newEventReservation();

        res.reservationFor.name = page.textInRect(0.0, 0, 0.5, 0.15);
        res.reservationFor.startDate = JsonLd.toDateTime(page.textInRect(0.0, 0.15, 0.5, 0.18), "dd.MM.yyyy", "sk") // 29.11.2024
        res.reservationNumber = page.textInRect(0.5, 0, 1, 0.3).replace("Tisk Colosseum", "").trim()
        res.reservedTicket.name = page.textInRect(0, 0.16, 0.6, 0.2)

        let _price = page.textInRect(0.5, 0.3, 1, 0.5).replace("Cena: ").split(" ") // [ "5", "EUR" ]
        res.totalPrice = Number(_price[0])
        res.priceCurrency = _price[1]

        let barcode = node.findChildNodes({ mimeType: "text/plain", match: ".*", scope: "Descendants" }).filter(n => n.parent.mimeType == "internal/qimage")[i]
        res.reservedTicket.ticketToken = 'qrCode:' + barcode.content

        results.push(res)
    }

    return results
}