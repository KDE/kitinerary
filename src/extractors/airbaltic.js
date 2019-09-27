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
