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
