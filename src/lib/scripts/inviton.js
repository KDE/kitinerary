// SPDX-FileCopyrightText: 2025 David Pilarčík <meow@charliecat.space>
// SPDX-License-Identifier: LGPL-2.0-or-later

const i18n = {
    sk: {
        __lookingFor: "Vstupenka:",
        printOutNoticeString: "Vstupenku si vytlačte a prineste na podujatie, predídete tým komplikáciam",
        eventName: /(.*)(?:Začiatok:|Kedy:)/,
        startTime: /(?:Začiatok:|Kedy:)\s+(.*)/,
        venueName: /(?:Kde:)\s+(.*)/,
        ticketType: /Vstupenka:\s+Vstupenka #\n(.*)\s+[A-Z0-9]{18}\n/,
    }
}

function parsePdf(pdf, node, triggerNode) {
    const page = pdf.pages[triggerNode.location];
    const res = JsonLd.newEventReservation();

    let lang = i18n["sk"]

    res.reservedTicket.ticketToken = "qrCode:" + triggerNode.content
    res.reservationNumber = triggerNode.content

    // spliting and joining the text, due to too long event names that new line.
    res.reservationFor.name = lang.eventName.exec(page.text.split("\n").join(" "))[1].replace(lang.printOutNoticeString, "")
    // in example: "09.09.2024 17:00"
    res.reservationFor.startDate = JsonLd.toDateTime(lang.startTime.exec(page.text)[1], 'dd.MM.yyyy hh:mm', 'sk')
    // in example: "Kde: Gopass Aréna, Bratislava"
    res.reservationFor.location.name = lang.venueName.exec(page.text)[1]
    // in example:
    // Vstupenka:
    // vstupenka
    res.reservedTicket.name = lang.ticketType.exec(page.text)[1]

    ExtractorEngine.extractPrice(page.text, res);

    return res
}

function parsePkPass(pass, node) {
    const res = Object.assign(JsonLd.newBusReservation(), node.result[0]);
    res.reservationNumber = pass.barcodes[0].message
    res.reservedTicket.name = pass.field['ticket-type'].value;

    return res
}