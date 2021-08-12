/*
   SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
   SPDX-License-Identifier: LGPL-2.0-or-later
*/

function parsePass(pass, node) {
    var res = node.result[0];
    res.reservationFor.startDate = pass.field["when"].value;
    res.underName = JsonLd.newObject("Person");
    res.underName.name = pass.field["buyer"].value;

    var addr = pass.field["venue"].value.match(/(.*), (.*)/);
    res.reservationFor.location.name = null;
    res.reservationFor.location.address = JsonLd.newObject("PostalAddress");
    res.reservationFor.location.address.addressCountry = addr[2];
    if (addr[2] == "Deutschland") {
        var addr2 = addr[1].match(/(.*) (\d{5} .*) ([A-Z]{2})/);
        res.reservationFor.location.address.streetAddress = addr2[1];
        res.reservationFor.location.address.addressLocality = addr2[2];
        res.reservationFor.location.address.addressRegion = addr2[3];
    } else {
        res.reservationFor.location.address.streetAddress = addr[1];
    }
    return res;
}
