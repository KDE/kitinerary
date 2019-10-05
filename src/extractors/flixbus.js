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

function main(content) {
    // convert QR download links into the actual QR codes
    var res = Context.data;
    for (var i = 0; i < res.length; ++i) {
        var ticketToken = res[i].reservedTicket.ticketToken;
        res[i].reservedTicket.ticketToken = ticketToken.replace(/^https?:\/\/api\.meinfernbus\.(..)\/qrcode\/..\//, "qrCode:https://shop.flixbus.$1/pdfqr/");
    }
    return res;
}
