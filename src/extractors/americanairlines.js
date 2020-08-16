/*
   SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

function main(pdf) {
    var res = Context.data[0];
    var page = pdf.pages[Context.pdfPageNumber];
    var time = page.text.match(/Departing at\s+(\d{1,2}:\d{2}[AP]M)/);
    if (time)
        res.reservationFor.departureTime = JsonLd.toDateTime(time[1], "h:mmA", "en")
    time = page.text.match(/Arriving at:\s+(\d{1,2}:\d{2}[AP]M)/);
    if (time)
        res.reservationFor.arrivalTime = JsonLd.toDateTime(time[1], "h:mmA", "en")
    return res;
}
