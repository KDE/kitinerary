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

function parseBarcode(barcode)
{
    var ticket = Barcode.decodeUic9183(barcode);
    if (!ticket)
        return null;

    var res = JsonLd.newTrainReservation();
    res.reservationNumber = ticket.pnr;
    res.reservedTicket.ticketToken = "aztectBin:" + Barcode.toBase64(barcode);
    res.underName.name = ticket.ticketLayout.text(0, 0, 72, 1);
    res.reservationFor.departureTime = JsonLd.toDateTime(ticket.ticketLayout.text(3, 0, 72, 1).match(/([\d\.: ]+)/)[1], "dd.MM.yyyy hh:mm", "de");
    res.reservationFor.departureStation.name = ticket.ticketLayout.text(7, 0, 72, 1);
    res.reservationFor.arrivalStation.name = ticket.ticketLayout.text(8, 0, 72, 1);
    return res;
}

function parsePdf(pdf) {
    // try to find the UIC918.3 barcode
    var images = pdf.pages[0].images;
    for (var i = 0; i < images.length; ++i) {
        var barcode = Barcode.decodeAztecBinary(images[i]);
        if (barcode)
            return parseBarcode(barcode);
    }

    return null;
}
