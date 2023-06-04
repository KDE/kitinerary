// SPDX-FileCopyrightText: 2023 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: LGPL-2.0-or-later

function parsePdfTicket(pdf, node, triggerNode) {
    const page = pdf.pages[triggerNode.location];

    const leftSidebarText = page.textInRect(0, 0.3, 0.3, 1).split('\n');

    const res = JsonLd.newEventReservation();
    res.reservationNumber = triggerNode.content;
    res.reservationFor.location.name = "Kino " + leftSidebarText[6];
    res.reservationFor.location.address.streetAddress = leftSidebarText[7];
    res.reservationFor.location.address.addressLocality = leftSidebarText[8];

    const mainText = page.textInRect(0.3, 0, 1, 1);

    const dt = mainText.match(/Datum (.*) \w* Zeit (.*) Uhr/);
    res.reservationFor.startDate = JsonLd.toDateTime(dt[1].trim() + ' ' + dt[2].trim(), 'dd.MM.yyyy hh:mm', 'de');

    res.underName = mainText.match(/Gebucht von (.*)/)[0];
    res.reservationFor.name = mainText.split('\n').join(' ').match(/Ticket\w*(.*)\w*Kino .* Datum .* Zeit/)[1]

    const seat = mainText.replace('\n', ' ').match(/Saal\w*(.*)\w*Reihe\w*(.*)\w*Platz\w*(.*)\w*/)

    res.reservedTicket.ticketedSeat.seatRow = seat[2];
    res.reservedTicket.ticketedSeat.seatNumber = seat[3];
    res.reservedTicket.ticketToken = 'qrCode:' + triggerNode.content;

    return res;
}
