/*
   Copyright (c) 2019 Nicolas Fella <nicolas.fella@gmx.de>

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

function main(pass)
{
    var res = JsonLd.newEventReservation();
    res.reservationFor.startDate = JsonLd.toDateTime(pass.field["reservation_time_from"].value, "dd.MM.yyyy", "de");
    res.reservationFor.endDate = JsonLd.toDateTime(pass.field["reservation_time_till"].value, "dd.MM.yyyy", "de");

    res.reservationFor.location.name = pass.field["logo_text"].value;
    var addr = pass.field["adsress"].value.split("\n"); // sic!
    res.reservationFor.location.address.streetAddress = addr[0];
    res.reservationFor.location.address.addressLocality = addr[1];
    res.reservationFor.location.address.addressCountry = addr[2];
    return res;
}

