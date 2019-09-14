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
    var result = new Array();
    
    for (var i = 0; i < pdf.pageCount; ++i) {
        var page = pdf.pages[i];
        var images = page.images;
        for (var j = 0; j < images.length; ++j) {
            var barcode = Barcode.decodeDataMatrix(images[j]);
            if (!barcode)
                continue;

            // barcode content:
            // 13x ticket number
            // 5x Renfe departure station id
            // 5x Renfe arrival station id
            // 6x departure(?) date: ddMMyy
            // 5x train number
            // 3x coach number
            // 3x seat number
            // 3x unknown number
            // 6x "localizador" ~ PNR?
            // ".." (optional)
            // 5x "CombinadoCercanias" (optional)
            
            var res = JsonLd.newTrainReservation();
            res.reservedTicket.ticketToken = "dataMatrix:" + barcode;
            res.reservationFor.trainNumber = barcode.substr(29, 5);
            res.reservedTicket.ticketedSeat.seatSection = barcode.substr(34, 3);
            res.reservedTicket.ticketedSeat.seatNumber = barcode.substr(37, 3);
            res.reservationNumber = barcode.substr(43);
            
            var text = page.text;
            var dep = text.match(/Salida +(.*?) {2,}([\d\/]+) +(\d\d:\d\d)/);
            res.reservationFor.departureStation.name = dep[1];
            res.reservationFor.departureTime = JsonLd.toDateTime(dep[2] + dep[3], "dd/MM/yyyyhh:mm", "es");
            
            var arr = text.match(/Llegada\s+(.*?) {2,}([\d\/]+) +(\d\d:\d\d)\n *(\S+) /);
            res.reservationFor.arrivalStation.name = arr[1];
            res.reservationFor.arrivalTime = JsonLd.toDateTime(arr[2] + arr[3], "dd/MM/yyyyhh:mm", "es");
            res.reservationFor.trainName = arr[4];
            
            result.push(res);
            break;
        }
    }
    
    return result;
}
