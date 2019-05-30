/*
   Copyright (c) 2019 Volker Krause <vkrause@kde.org>

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

function parseEvent(event)
{
    var res;
    res = JsonLd.newObject("FoodEstablishmentReservation");
    res.reservationFor = JsonLd.newObject("FoodEstablishment");
    res.reservationFor.name = event.organizer.name;
    res.startTime = event.dtStart.toJSON();
    res.endTime = event.dtEnd.toJSON();
    res.reservationFor.address = JsonLd.newObject("PostalAddress");
    var addr = event.location.split(', ');
    res.reservationFor.address.streetAddress = addr[1];
    res.reservationFor.address.addressLocality = addr[2];

    var country = event.description.match(addr[2] + "\\n([\\w ]+)\\n\\n");
    if (country)
        res.reservationFor.address.addressCountry = country[1];

    var cancelUrl = event.description.match(/\n(https?:\/\/.*?\/cancel.*?)\n/);
    if (cancelUrl) {
        res.potentialAction = JsonLd.newObject("CancelAction");
        res.potentialAction.url = cancelUrl[1];
    }

    var url = event.description.match(/\n(https?:\/\/.+?)\n$/);
    if (url)
        res.reservationFor.url = url[1];

    return res;
}
