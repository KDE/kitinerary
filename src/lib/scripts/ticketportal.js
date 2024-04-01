// SPDX-FileCopyrightText: 2024 David Pilarčík <meow@charliecat.space>
// SPDX-License-Identifier: LGPL-2.0-or-later

function hometicket(content) {

    const reservations = []

    let codeRegex = /\s+KOPIE [0-9A-Z-]{36}\n\s+([0-9]{6})\s+([0-9]{10})\n\s+eTicket číslo:/ // [Reservation Number, Ticket Token]
    let priceRegex = /eTicket číslo:\n\s+([0-9]+.[0-9]+.[0-9]+ od [0-9]+:[0-1]+ hod.)\n\s+(.*?)\n\s+Cena: ([0-9,]+ Kč|€)/ // [Start Date, Location Name, Price]
    let organizatorRegex = /Pořadatel: (.*?); IČ: [0-9]+; DIČ: [A-Z0-9]+; Cena .*\n/ // [Organizer]

    for (page of content.pages) {
        let res = JsonLd.newEventReservation();

        let code = codeRegex.exec(page.text)
        let datePlacePrice = priceRegex.exec(page.text)
        let organizer = organizatorRegex.exec(page)

        res.reservationFor.location.name = datePlacePrice[2]
        res.reservationFor.startDate = JsonLd.toDateTime(datePlacePrice[1], "dd.MM.yyyy od hh:mm hod.", "cz") // 11.10.2022 od 19:00 hod.

        res.priceCurrency = datePlacePrice[3].includes("Kč") ? "CZK" : "EUR"
        res.totalPrice = Number(datePlacePrice[3].replace(/Kč|Eur|€/, "").replace(",", ".").replace("-", 0))

        res.reservedTicket.ticketToken = "qrCode:" + code[2]
        res.reservationNumber = code[1]

        res.provider = { '@type': 'Organization', name: organizer[1] };

        reservations.push(res)
    }
    

    return reservations
}

function pkpassTicket(pass, node) {

    const res = node.result[0];

    console.log(JSON.stringify(pass.field, null, 4))

    res.reservationNumber = pass.field["af_2"].value

    res.reservationFor.name = pass.field["pf_0"].value
    res.reservationFor.startDate = JsonLd.toDateTime(pass.field["sf_0"].value, "d. MMMM yyyy hh:mm", "sk") // 7. augusta 2022 19:00

    res.reservationFor.location = JsonLd.newObject("Place");
    res.reservationFor.location.name = pass.field["af_0"].value;

    res.reservedTicket.name = pass.field["hf_0"].value

    /*
    // In sample: Nmiest / 0 / 0 
    // Needs verifying

    let seat = pass.field["af_1"].value.split(' / ')

    res.reservedTicket.ticketedSeat.seatSection = seat[0]
    res.reservedTicket.ticketedSeat.seatRow = seat[1]
    res.reservedTicket.ticketedSeat.seatNumber = seat[2]
    */

    return res
}