/*
    SPDX-FileCopyrightText: 2024 David Pilarčík <meow@charliecat.space>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

function parsePass(pass, node) {
    const res = Object.assign(JsonLd.newEventReservation(), node.result[0])

    res.reservationFor.startDate = JsonLd.toDateTime(pass.field['validity'].value, "dd.mm.yyyy", "sk") // 09.08.2024
    res.reservationFor.name = pass.field["ticket"].value ?? res.reservationFor.name

    res.underName = { '@type': 'Person', email: pass.field['email'].value }
    res.provider = { '@type': 'Organization', name: pass.field['seller'].value };
    res.reservationNumber = pass.barcodes[0].alternativeText


    return res
}

function parsePDF(pdf, node, barcode) {
    const result = []
    let ticketCounter = 0

    for (let i = 0; i < pdf.pages.length; i++) {
        const page = pdf.pages[i];
        let res = JsonLd.newEventReservation()
        if (!page.text.includes("DROP/TID/"))
            continue;

        // Ticket Title Example:
        // Tatrzański Park Narodowy
        // SAMOCHÓD OSOBOWY do 5 m długości - parkingi przy szlaku
        // do Morskiego Oka
        let ticketNameAndType = page.textInRect(0.0, 0, 1, 0.13).split("\n").slice(1).join(" ").split(", ")
        res.reservationFor.name = ticketNameAndType[0]
        res.reservedTicket.name = ticketNameAndType[1] ?? undefined

        let dateAndTime = page.textInRect(0.0, 0.1, 1, 0.15).match(/.*: (\d{2}.\d{2}.\d{4}).*(\d*:*\d*)/) // [ 09.08.2024, 20:00 ] or [ 09.08.2024 ]
        if (dateAndTime[2].length != 0)
            res.reservationFor.startDate = JsonLd.toDateTime(dateAndTime[1] + " " + dateAndTime[2], "dd.MM.yyyy hh:mm", "pl")
        else
            res.reservationFor.startDate = JsonLd.toDateTime(dateAndTime[1], "dd.MM.yyyy", "pl")

        let locationDetect = page.textInRect(0.0, 0.15, 1, 0.25).split("\n") // [Palenica Białczańska - Łysa Polana] or [Preskočenie frontu na lístky,Tatrzański Park Narodowy,]
        res.reservationFor.location.name = (locationDetect.length == 1) ? locationDetect[0] : locationDetect[1]

        let ticketPriceRegex = /.*: ([+-]?(?=,\d|\d)(?:\d+)?(?:,?\.?\d*))(?:[Ee]([+-]?\d+))? ([A-Za-z0-9]+)/
        let foundPrice = page.text.match(ticketPriceRegex)
        if (!foundPrice)
            foundPrice = pdf.pages[i+1].text.match(ticketPriceRegex)
        res.totalPrice = parseInt(foundPrice[1])
        res.priceCurrency = foundPrice[3]

        res.underName = { 
            '@type': 'Person', 
            email: page.text.match(/E-mail .*: ([-A-Za-z0-9!#$%&'*+\/=?^_`{|}~]+(?:\.[-A-Za-z0-9!#$%&'*+\/=?^_`{|}~]+)*@(?:[A-Za-z0-9](?:[-A-Za-z0-9]*[A-Za-z0-9])?\.)+[A-Za-z0-9](?:[-A-Za-z0-9]*[A-Za-z0-9])?)/i)[1]
        }
        res.reservationNumber = page.text.match(/DROP\/TID\/[A-Z0-9]+/)[0]

        let barcode = node.findChildNodes({ mimeType: "text/plain", match: ".*", scope: "Descendants" }).filter(n => n.parent.mimeType == "internal/qimage")[ticketCounter]
        res.reservedTicket.ticketToken = 'qrCode:' + barcode.content

        result.push(res)
        ticketCounter++
    }
    
    return result
}