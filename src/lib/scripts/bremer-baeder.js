/*
   SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
   SPDX-License-Identifier: LGPL-2.0-or-later
*/

function parseTicket(pdf, node) {
    var res = JsonLd.newEventReservation();
    var barcode = node.childNodes[0].childNodes[0].content;
    res.reservedTicket.ticketToken = "qrCode:" + barcode;
    var text = pdf.pages[0].text;
    var date = text.match(/Eintrittszeit: (\d{2}\.\d{2}\.\d{4} \d{2}:\d{2}) Austrittszeit: ((\d{2}\.\d{2}\.\d{4} \d{2}:\d{2}))/);
    res.reservationFor.startDate = JsonLd.toDateTime(date[1], "dd.MM.yyyy hh:mm", "de");
    res.reservationFor.endDate = JsonLd.toDateTime(date[2], "dd.MM.yyyy hh:mm", "de");
    res.reservationNumber = text.match(/Auftrag (\d+)/)[1];
    res.underName.givenName = text.match(/Vorname: (.*?)\n/)[1];
    res.underName.familyName = text.match(/Name: (.*?)\n/)[1];
    res.reservationFor.name = text.match(/ONLINE-TICKET\n *(.*?) -/)[1];
    res.reservationFor.location.address.addressCountry = "DE"; // to get the right timezone
    return res;
}
