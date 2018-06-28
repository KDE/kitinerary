/*
   Copyright (c) 2018 Volker Krause <vkrause@kde.org>

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

function main(pdf) {
    var result = new Array();

    for (var i = 0; i < pdf.pageCount; ++i) {
        var page = pdf.pages[i];
        var images = page.imagesInRect(0.6, 0.1, 1, 0.2);
        for (var j = 0; j < images.length; ++j) {
            var bcbp = Barcode.decodePdf417(images[j]);
            if (!bcbp)
                continue;
            result = result.concat(JsonLd.toJson(Barcode.decodeIataBcbp(bcbp)));
        }
    }

    return result;
}
