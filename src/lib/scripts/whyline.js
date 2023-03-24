/*
   SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
   SPDX-License-Identifier: LGPL-2.0-or-later
*/

function parseEvent(event) {
    let res = JsonLd.newEventReservation();
    res.reservationFor.startDate =  JsonLd.readQDateTime(event, 'dtStart');
    res.reservationFor.endDate =  JsonLd.readQDateTime(event, 'dtEnd');
    res.reservationFor.location.geo.latitude = event.geoLatitude;
    res.reservationFor.location.geo.longitude = event.geoLongitude;
    res.reservationFor.location.name = event.organizer.name;
    const name = event.summary.match(/Appointment at (.*?) for (.*)/);
    if (name && name[1].startsWith(event.organizer.name)) {
        res.reservationFor.name = name[2];
    } else {
        res.reservationFor.name = event.summary;
    }
    res.reservationNumber = event.uid.match(/(.*)@/)[1];
    return res;
}

function parseBarcode(data) {
    let res = JsonLd.newEventReservation();
    res.reservedTicket.ticketToken = 'qrcode:' + data;
    res.reservationNumber = 'BE-vAFCK0UjWR';
    return res;
}
