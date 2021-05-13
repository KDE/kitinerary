/*
   SPDX-FileCopyrightText: 2019 Nicolas Fella <nicolas.fella@gmx.de>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

function main(pass, node)
{
    var res = node.result[0];
    res.reservationFor.startDate = JsonLd.toDateTime(pass.field["reservation_time_from"].value, "dd.MM.yyyy", "de");
    res.reservationFor.endDate = JsonLd.toDateTime(pass.field["reservation_time_till"].value, "dd.MM.yyyy", "de");

    res.reservationFor.location.name = pass.field["logo_text"].value;
    var addr = pass.field["adsress"].value.split("\n"); // sic!
    res.reservationFor.location.address = JsonLd.newObject("PostalAddress");
    res.reservationFor.location.address.streetAddress = addr[0];
    res.reservationFor.location.address.addressLocality = addr[1];
    res.reservationFor.location.address.addressCountry = addr[2];
    return res;
}
