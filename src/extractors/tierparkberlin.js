/*
    SPDX-FileCopyrightText: 2020 Nicolas Fella <nicolas.fella@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

function main(input) {
    
    var page = input.pages[Context.pdfPageNumber];
    
    const lines = page.text.split("\n")
    
    const res = JsonLd.newEventReservation()
    res.underName = JsonLd.newObject("Person");
    
    let date = ""
    let startTime = ""
    let endTime = ""
    
    res.reservedTicket.ticketToken = "qrCode:" + Context.barcode
    
    const address = JsonLd.newObject("PostalAddress")
    address.addressCountry = "DE"
    address.addressLocality = "Berlin"
    address.addressRegion = "Berlin"
    address.postalCode = "10319"
    address.streetAddress = "Am Tierpark 125"
    
    res.reservationFor.location.address = address
    res.reservationFor.name = "Tierpark Berlin"
    
    for (line of lines) {
        
        const nameMatch = line.match(/Name:[ ]*([^\s].*)/)
        if (nameMatch) {
            res.underName.name = nameMatch[1]
        }
        
        const bookingCodeMatch = line.match(/Booking code:[ ]*([^\s].*)/)
        if (bookingCodeMatch) {
            res.reservationNumber = bookingCodeMatch[1]
        }
        
        const dateMatch = line.match(/Valid on:[ ]*([0-9]{1,2} [a-zA-Z]+ [0-9]{4})/)
        if (dateMatch) {
            date = dateMatch[1]
        }
        
        const timeMatch = line.match(/Time slot:[ ]*([0-9]{1,2}:[0-9]{1,2} (AM|PM)) - ([0-9]{1,2}:[0-9]{1,2} (AM|PM))/)
        if (timeMatch) {
            startTime = timeMatch[1]
            endTime = timeMatch[3]
        }
    }

    res.reservationFor.startDate = JsonLd.toDateTime(date + " " + startTime, "dd MMM yyyy h:mm ap", "en")
    res.reservationFor.endDate = JsonLd.toDateTime(date + " " + endTime, "dd MMM yyyy h:mm ap", "en")

    return [res]
}
