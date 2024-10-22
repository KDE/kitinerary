// SPDX-FileCopyrightText: 2024 Volker Krause <vkrause@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

function extractConfirmation(content) {
    let res = JsonLd.newEventReservation();
    res.reservationFor.name = content.match(/Service: (.*)/)[1];
    const time = content.match(/(\d\d\/\d\d\/\d\d\d\d) .* (\d\d:\d\d)/)
    res.reservationFor.startDate = JsonLd.toDateTime(time[1] + time[2], "dd/MM/yyyyhh:mm", "dk");
    const addr = content.match(/Adresse: (.*), (.*)/);
    res.reservationFor.location.address.streetAddress = addr[1];
    res.reservationFor.location.address.addressLocality = addr[2];
    return res;
}
