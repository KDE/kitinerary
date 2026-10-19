// SPDX-FileCopyrightText: 2026 Volker Krause <vkrause@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

function extractEvent(ev) {
    console.log(ev);
    let res = JsonLd.newFoodEstablishmentReservation();
    res.reservationFor.name = ev.summary;
    const addr = ev.location.split(/, /);
    res.reservationFor.address.addressLocality = addr[addr.length - 1];
    addr.pop();
    res.reservationFor.address.streetAddress = addr.join(", ");
    res.startTime = JsonLd.readQDateTime(ev, "dtStart");
    res.endTime = JsonLd.readQDateTime(ev, "dtEnd");
    return res;
}
