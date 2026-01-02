// SPDX-FileCopyrightText: 2024 Volker Krause <vkrause@kde.org>
//SPDX-License-Identifier: LGPL-2.0-or-later

function extractPdf(pdf, node, triggerNode) {
    const text = pdf.pages[triggerNode.location].text;

    let res = JsonLd.newEventReservation();
    const ev = text.match(/(.*)\n+(\d{4}\.\d{2}\.\d{2}) - (\d{4}\.\d{2}\.\d{2})\n+.*?: (.*)\n+.*?: (.*)\n/);

    res.reservationFor.name = ev[1];
    res.reservationFor.startDate = JsonLd.toDateTime(ev[2], "yyyy.MM.dd", "pl");
    res.reservationFor.endDate = JsonLd.toDateTime(ev[3], "yyyy.MM.dd", "pl");
    res.reservationFor.location.address.addressLocality = ev[4];
    res.reservationFor.location.name = ev[5];
    res.reservedTicket.ticketToken = "qrCode:" + triggerNode.content;

    return res;
}
