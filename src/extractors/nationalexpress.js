/*
   SPDX-FileCopyrightText: 2019 Nicolas Fella <nicolas.fella@gmx.de>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

function parseHtmlBooking(doc) {
    
    var res = JsonLd.newBusReservation()

    var baseTable = "//tbody/tr/td/table/tr/td/table"

    res.reservationNumber = doc.eval(baseTable + "/tr/td/p/strong")[0].content
    
    res.underName.name = doc.eval(baseTable + "[4]/tr/td[2]/p/strong")[0].content
    
    res.reservationFor.departureBusStop.name = doc.eval(baseTable + "[6]/tr[1]/td/h3/span")[1].content
    
    res.reservationFor.arrivalBusStop.name = doc.eval(baseTable + "[6]/tr[1]/td/h3/span")[2].content
    
    var theDate = doc.eval(baseTable + "[6]/tr[3]/td/table/tr[2]/td[1]/strong")[0].content
    
    var depTime = doc.eval(baseTable + "[6]/tr[3]/td/table/tr[2]/td[2]/strong")[0].content
    
    var arrTime = doc.eval(baseTable + "[6]/tr[3]/td/table/tr[2]/td[3]/strong")[0].content
    
    res.reservationFor.departureTime = makeDateTime(theDate, depTime)

    res.reservationFor.arrivalTime = makeDateTime(theDate, arrTime)
    
    res.reservationFor.busNumber = doc.eval(baseTable + "/tr/td/strong")[0].content
    
    var barcodeUrlSplit = doc.eval("//img[@alt=\"Bar Code\"]")[0].attribute("src").split("/")
    
    res.reservedTicket.ticketToken = "qrCode:" + decodeURIComponent(barcodeUrlSplit[barcodeUrlSplit.length - 1])
    
    return res
}

function makeDateTime(theDate, theTime) {
    return JsonLd.toDateTime(theDate + " " + theTime, "ddd MMM dd hh:mm", "en")
}
