/*
   SPDX-FileCopyrightText: 2025 Kai Uwe Broulik <kde@broulik.de>
   SPDX-License-Identifier: LGPL-2.0-or-later
*/

function parseReservation(pass, node) {
    let res = node.result[0];
    res.reservationFor.name = pass.organizationName;
    res.underName = JsonLd.newObject("Person");
    res.underName.name = pass.field["person"].value;
    res.reservationNumber = pass.field["code"].value;

    const linkField = pass.field["link"];
    if (linkField) {
        res.potentialAction = JsonLd.newObject("UpdateAction");
        res.potentialAction.url = linkField.value;
    }
    return res;
}
