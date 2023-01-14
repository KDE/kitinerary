/*
   SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
   SPDX-License-Identifier: LGPL-2.0-or-later
*/

function parseEvent(event) {
    let res = JsonLd.newEventReservation().reservationFor;
    res.name = event.summary;
    res.startDate = JsonLd.readQDateTime(event, 'dtStart');
    res.endDate = JsonLd.readQDateTime(event, 'dtEnd');
    const loc = event.location.match(/(.*) \((.*)\)/);
    if (loc) {
        res.location.name = loc[1];
        const addr = loc[2].split(', ');
        res.location.address.addressCountry = addr[addr.length - 1];
        if (addr.length >= 4) {
            res.location.address.addressRegion = addr[addr.length - 2];
        }
        res.location.address.addressLocality = addr[1];
        res.location.address.streetAddress = addr[0];
    } else {
        res.location.name = event.location;
    }
    res.location.geo.latitude = event.geoLatitude;
    res.location.geo.longitude = event.geoLongitude;
    res.description = event.description;
    res.url = event.url;
    return res;
}
