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

function main(doc) {

    var reservations = new Array()

    var itineraryTable = doc.eval("//table/tr/td/div/div/div/table")[0]

    // First child is the header
    var item = itineraryTable.firstChild.nextSibling

    while (!item.isNull) {

        var res = JsonLd.newTrainReservation()

        res.reservationNumber = doc.eval("//table/tr/td/table[1]/tr[1]/td[2]")[0].content
        res.underName.name = doc.eval("//table/tr/td/table[1]/tr[2]/td[2]")[0].content

        res.reservationFor.departureStation.name = item.eval("td[1]")[0].content
        res.reservationFor.arrivalStation.name = item.eval("td[2]")[0].content

        var depDate = item.eval("td[3]")[0].content
        var arrDate = item.eval("td[4]")[0].content

        res.reservationFor.departureTime = JsonLd.toDateTime(depDate, "yyyy-MM-dd hh:mm", "en")
        res.reservationFor.arrivalTime = JsonLd.toDateTime(arrDate, "yyyy-MM-dd hh:mm", "en")

        res.reservationFor.trainNumber = item.eval("td[5]")[0].content

        reservations.push(res)

        item = item.nextSibling
    }

    var cont = true
    var idx = 2
    // Reserved seats come after the itinerary block and there might not be a reserved seat for all legs, so we need to match the reservations to the legs
    while (cont) {
        var reservationBlock = doc.eval("//table/tr/td/div/div/div")[idx]

        var title = reservationBlock.eval("span/strong")[0].content

        // Unfortunately we cannot know when to stop from the DOM structure
        if (!title.includes("Seating")) {
            cont = false
        }

        var departureStation = title.split("\n")[1]

        // Find from itinerary based on station names
        for (res of reservations) {
            if (departureStation == res.reservationFor.departureStation.name + " - " + res.reservationFor.arrivalStation.name) {
                var seat = reservationBlock.eval("table/tr[2]/td[2]")[0].content
                var coach = reservationBlock.eval("table/tr[2]/td[3]")[0].content

                res.reservedTicket.ticketedSeat.seatNumber = reservationBlock.eval("table/tr[2]/td[2]")[0].content
                res.reservedTicket.ticketedSeat.seatSection = reservationBlock.eval("table/tr[2]/td[3]")[0].content

                console.log(seat, coach)
            }
        }
        idx++
    }

    return reservations
}
