/*
   SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

function main(pdf)
{
    var res = Context.data[0];
    var page = pdf.pages[Context.pdfPageNumber];
    var time = page.text.match(/Boarding\s+(\d{1,2}.\d{1,2}.\d{4})[\s.]+?(\d{2}:\d{2})/);
    if (time) {
        res.reservationFor.boardingTime = JsonLd.toDateTime(time[1] + time[2], "dd.MM.yyyyhh:mm", "en")
        res.reservationFor.departureDay = ""; // reset departure day from IATA BCBP, we know better now
    }
    time = page.text.match(/Arr. terminal\s+(\d{2}:\d{2})\s+(\d{2}:\d{2})/);
    if (time) {
        res.reservationFor.departureTime = JsonLd.toDateTime(time[1], "hh:mm", "en")
        res.reservationFor.arrivalTime = JsonLd.toDateTime(time[2], "hh:mm", "en")
    }
    return res;
}
