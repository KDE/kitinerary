/*
    Copyright (C) 2020 Volker Krause <vkrause@kde.org>

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

function parseConfirmation(html) {
    var res = JsonLd.newLodgingReservation();

    var elems = html.eval('//table//table//table//table//table//td[@align="right"]/..');
    for (var i = 0; i < elems.length; ++i) {
        var title = elems[i].firstChild.content;
        if (title.match(/(Reservation number|Buchungsnummer)/i)) {
            res.reservationNumber = elems[i].firstChild.nextSibling.recursiveContent;
        }
        if (title.match(/(Date of stay|Aufenthaltsdatum)/i)) {
            var dt = elems[i].firstChild.nextSibling.recursiveContent.match(/(\d{2}([|/.])\d{2}[|/.]\d{4}).*(\d{2}[|/.]\d{2}[|/.]\d{4})/);
            if (dt) {
                var separator = dt[2];
                var format = ["dd", "MM", "yyyy"].join(separator);
                var lang = (separator === "." ? "de" : "en");

                res.checkinTime = JsonLd.toDateTime(dt[1], format, lang);
                res.checkoutTime = JsonLd.toDateTime(dt[3], format, lang);
            }
        }
    }

    var hotelContent = html.eval('//table//table//table[@class="table-full"]')[0].recursiveContent;
    hotelContent = hotelContent.replace(/\s+\n/, "\n");

    var hotel = hotelContent.match(/^(.*)\n+(.*\n.*)\n(?:.|\n)*Tel\s*:\s*([\d \/\+\(\)]+)\n(.+@.+?)[\s\n]/);
    res.reservationFor.name = hotel[1];
    res.reservationFor.telephone = hotel[3];
    res.reservationFor.email = hotel[4];

    var addr = hotel[2].match(/(.*)(?: - |\n)(.*) - (.*)/);
    res.reservationFor.address.streetAddress = addr[1];
    res.reservationFor.address.addressLocality = addr[2];
    res.reservationFor.address.addressCountry = addr[3];

    return res;
}
