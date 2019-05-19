/*
   Copyright (c) 2019 Volker Krause <vkrause@kde.org>

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

function parsePdf(pdf) {
    var reservations = new Array();

    for (var i = 0; i < pdf.pageCount; ++i) {
        var page = pdf.pages[i];
        var images = page.images;
        for (var j = 0; j < images.length; ++j) {
            var barcode = Barcode.decodeQR(images[j]);
            if (!barcode)
                continue;
            var obj = JSON.parse(barcode);
            if (!obj)
                continue;

            var res = JsonLd.newObject("TrainReservation");
            res.reservedTicket = JsonLd.newObject("Ticket");
            res.reservationFor = JsonLd.newObject("TrainTrip");
            res.reservationFor.departureStation = JsonLd.newObject("TrainStation");
            res.reservationFor.arrivalStation = JsonLd.newObject("TrainStation");
            res.underName = JsonLd.newObject("Person");

            res.underName.givenName = obj.name;
            res.underName.familyName = obj.surname
            res.reservationNumber = obj.idDocValue;
            res.reservationFor.trainNumber = obj.nrKursu;
            res.reservationFor.departureStation.name = obj.fromStop;
            res.reservationFor.arrivalStation.name = obj.toStop;
            var depDate = new Date();
            depDate.setTime(obj.goDate);
            res.reservationFor.departureTime = depDate;

            reservations.push(res);
        }
    }

    return reservations;
}
