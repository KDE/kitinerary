/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
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
            const dtStr = elems[i].firstChild.nextSibling.recursiveContent;
            var dt = dtStr.match(/(\d{2}[|/.]\d{2}[|/.]\d{4}).*(\d{2}[|/.]\d{2}[|/.]\d{4})/);
            if (!dt) { // US format
                dt = dtStr.match(/([A-Z][a-z]{2}\. \d{2}, \d{4}).*([A-Z][a-z]{2}\. \d{2}, \d{4})/);
            }
            if (dt) {
                var separator = dt[2];
                const formats = [ "dd.MM.yyyy", "dd/MM/yyyy", "MMM. dd, yyyy"];
                res.checkinTime = JsonLd.toDateTime(dt[1], formats, "en");
                res.checkoutTime = JsonLd.toDateTime(dt[2], formats, "en");
            }
        }
    }

    var hotelContent = html.eval('//table//table//table[@class="table-full"]')[0].recursiveContent;
    hotelContent = hotelContent.replace(/\s+\n/, "\n");

    var hotel = hotelContent.match(/^(.*)\n+(.*\n.*)\n(?:.|\n)*Tel\s*:\s*([\d \/\+\(\)]+)\n(.+@.+?)[\s\n]/);
    if (hotel) {
        res.reservationFor.telephone = hotel[3];
        res.reservationFor.email = hotel[4];
    } else {
        // no contact information present
        hotel = hotelContent.match(/^(.*)\n+(.*\n.*)\n(?:.|\n)*/);
    }
    res.reservationFor.name = hotel[1];


    var addr = hotel[2].match(/(.*)(?: - |\n)(.*) - (.*)/);
    res.reservationFor.address.streetAddress = addr[1];
    res.reservationFor.address.addressLocality = addr[2];
    res.reservationFor.address.addressCountry = addr[3];

    // this is in an HTML comment...
    if (html.rawData.match(/TARS - AH - Client - Cancellation - Email/)) {
        res.reservationStatus = "ReservationCancelled"
    }

    return res;
}
