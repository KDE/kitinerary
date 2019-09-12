/*
   Copyright (c) 2019 Nicolas Fella <nicolas.fella@gmx.de>

   This library is free software; you can redistribute it and/or modify it
   under the terms of the GNU Library General Public License as published by
   the Free Software Foundation; either version 2 of the License, or (at your
   option) any later version.

   This library is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
   FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
   License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to the
   Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
   02110-1301, USA.
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
