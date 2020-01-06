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

function parseReservation(html) {
    var res = JsonLd.newTrainReservation();

    var subtitle = html.eval('//table[@class="subtitle"]');
    var ref = subtitle[0].recursiveContent.match(/(\d{2}.\d{2}.\d{4})[\s\S]*([A-Z0-9]{6})/);
    res.reservationNumber = ref[2];

    var schedule = html.eval('//table[@class="schedule"]')[0].eval(".//tr");
    var stations = schedule[1].recursiveContent.match(/(.*)\n.*\n(.*)/);
    res.reservationFor.departureStation.name = stations[1];
    res.reservationFor.arrivalStation.name = stations[2];

    var times = schedule[2].recursiveContent.match(/(\d{2}:\d{2})[\s\S]*(\d{2}:\d{2})/);
    res.reservationFor.departureTime = JsonLd.toDateTime(ref[1] + times[1], "dd/MM/yyyyhh:mm", "en");
    res.reservationFor.arrivalTime = JsonLd.toDateTime(ref[1] + times[2], "dd/MM/yyyyhh:mm", "en");

    var detailsElem = html.eval('//table[@class="detailtrain"]')[0];
    var details = detailsElem.recursiveContent.match(/(\d{4})\n[\s\S]*?(\d{1})\n[\s\S]*?(\d{1,2})\n[\s\S]*?(\d{1,3})/);
    res.reservationFor.trainNumber = "THA " + details[1];
    res.reservedTicket.ticketedSeat.seatingType = details[2];
    res.reservedTicket.ticketedSeat.seatSection = details[3];
    res.reservedTicket.ticketedSeat.seatNumber = details[4];

    var tokenElem = html.eval('//table[@class="qrcode"]//img')[0];
    var token = tokenElem.attribute("src").match(/barcode\/tAZTEC\/.*\/nBinary\/v(.*)\/barcode.gif/);
    res.reservedTicket.ticketToken = "aztecbin:" + token[1];

    var passengerElem = html.eval('//table[@class="passengername"]')[0];
    var name = passengerElem.recursiveContent.match(/\n(.*)/);
    res.underName.name = name[1];

    return res;
}
