/*
   SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

function main(content, node) {
    // convert QR download links into the actual QR codes
    var res = node.result;
    for (var i = 0; i < res.length; ++i) {
        var ticketToken = res[i].reservedTicket.ticketToken;
        res[i].reservedTicket.ticketToken = ticketToken.replace(/^https?:\/\/api\.meinfernbus\.(..)\/qrcode\/..\//, "qrCode:https://shop.flixbus.$1/pdfqr/");
    }
    return res;
}
