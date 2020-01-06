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

// see https://community.kde.org/KDE_PIM/KItinerary/vr.fi_Barcode
function parseTicket(pdf) {
    var res = JsonLd.newTrainReservation();
    var bitarray = Barcode.toBitArray(Context.barcode);

    var trainNum = bitarray.readNumberMSB(22 * 8, 14) + "";
    if (trainNum == 0)
        return null; // TODO this misses bus legs!

    var text = pdf.pages[Context.pdfPageNumber].text;
    var trip = text.match("(.*) - (.*)\n.*(\\d{4}).*?(\\d{2}:\\d{2}).*?(\\d{2}:\\d{2})\n(.*?" + trainNum + ")");
    res.reservationFor.trainNumber = trip[6];

    var departureDay = bitarray.readNumberMSB(4 * 8 + 7, 9) - 1;
    var day = new Date(0);
    day.setYear(trip[3]);
    day.setTime(day.getTime() + departureDay * 24 * 60 * 60 * 1000);
    res.reservationFor.departureTime = JsonLd.toDateTime(day.getFullYear() + '-' + (day.getMonth() + 1) + '-' + day.getDate() + '-' + trip[4], "yyyy-M-d-hh:mm", "en");
    res.reservationFor.arrivalTime = JsonLd.toDateTime(day.getFullYear() + '-' + (day.getMonth() + 1) + '-' + day.getDate() + '-' + trip[5], "yyyy-M-d-hh:mm", "en");

    var coachNumber = bitarray.readNumberMSB(30 * 8, 6);
    if (coachNumber > 0) {
        res.reservedTicket.ticketedSeat.seatSection = coachNumber + "";
        res.reservedTicket.ticketedSeat.seatNumber = bitarray.readNumberMSB(30 * 8 + 6, 7) + "";
    }

    res.reservationFor.departureStation.name = trip[1];
    res.reservationFor.arrivalStation.name = trip[2];

    res.reservedTicket.ticketToken = "aztectbin:" + Barcode.toBase64(Context.barcode);
    return res;
}
