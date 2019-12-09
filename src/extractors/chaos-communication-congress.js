/*
    Copyright (C) 2019 Volker Krause <vkrause@kde.org>

    This program is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This program is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

function parsePdf(pdf)
{
    if (!Context.data) {
        return null;
    }

    var ptTicket = Barcode.decodeUic9183(Context.barcode);
    var res = JsonLd.newEventReservation();
    res.reservationFor.name = ptTicket.person.name;
    res.reservationFor.location.name = "Congress Center Leipzig";
    res.reservationFor.location.address.addressLocality = "Leipzig";
    res.reservationFor.location.address.addressCountry = "DE";
    res.reservationFor.location.geo.latitude = 51.39737;
    res.reservationFor.location.geo.longitude = 12.39528;
    var start = ptTicket.rct2Ticket.firstDayOfValidity;
    start.setHours(11);
    res.reservationFor.startDate = start;
    var end = ptTicket.rct2Ticket.firstDayOfValidity;
    end.setDate(30);
    end.setHours(18);
    res.reservationFor.endDate = end;

    // search for the QR code with the actual event code
    var images = pdf.pages[Context.pdfPageNumber].images;
    for (var i = 0; i < images.length; ++i) {
        var code = Barcode.decodeQR(images[i]);
        if (code) {
            res.reservedTicket.ticketToken = "qrcode:" + code;
            break;
        }
    }

    // generate the second ticket for public transport
    var pt = JsonLd.clone(res);
    pt.reservedTicket.ticketToken = Context.data[0].reservedTicket.ticketToken;
    pt.reservedTicket.name = "Public Transport";

    var reservations = new Array();
    reservations.push(res);
    reservations.push(pt);
    return reservations;
}
