/*
   SPDX-FileCopyrightText: 2021 Kai Uwe Broulik <kde@broulik.de>
   SPDX-License-Identifier: LGPL-2.0-or-later
*/

function parseReservation(pass, node) {
    let res = node.result[0];

    const secondary = pass.secondaryFields;
    const cardName = secondary.find(item => item.key === "cardName");
    const ausgabestelleName = secondary.find(item => item.key === "ausgabestelleName");

    if (cardName) {
        res.underName = JsonLd.newObject("Person");
        res.underName.name = cardName.value;
    }
    if (ausgabestelleName) {
        res.reservationFor.name = ausgabestelleName.value;
    }

    const aux = pass.auxiliaryFields;
    const cardValidFrom = aux.find(item => item.key === "cardValidFrom");
    const cardValidTo = aux.find(item => item.key === "cardValidTo");

    if (cardValidFrom) {
        res.reservationFor.startDate = cardValidFrom.value;
    }
    if (cardValidTo) {
        res.reservationFor.endDate = cardValidTo.value;
    }

    const back = pass.backFields;
    const web = back.find(item => item.key === "web");

    if (web) {
        res.reservationFor.url = web.value;
    }

    return res;
}
