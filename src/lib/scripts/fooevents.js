// SPDX-FileCopyrightText: 2025 David Pilarčík <meow@charliecat.space>
// SPDX-License-Identifier: LGPL-2.0-or-later

// https://demo.fooevents.com

const i18n = {
    sk: {
        __lookingFor: "",
        "Ticket Holder": /Držiteľ lístka: (.*)/,
        "Venue name": /Miesto: (.*)/,
        "Date": /Dátum: (.*)/,
        dateFormats: [
            "d. MMMM yyyy h:mm t",
            "d. MMMM yyyy "
        ],
        startDateFormatRegex: /(\d{1,2}. [a-z]+ \d{4}) \s* ?(\d{1,2}:\d{2} ?(?:[A-Z]{2,})?)?/,
        endDateFormatRegex: /- (\d{1,2}. [a-z]+ \d{4})? ?\s* ?(\d{1,2}:\d{2} ?(?:[A-Z]{2,})?)?/
    },
    en: {
        __lookingFor: "",
        "Ticket Holder": /Ticket Holder: (.*)/,
        "Venue name": /Venue: (.*)/,
        "Date": /Date: \s+ (.*)/,
        "Ticket Type": /Ticket Type: (.*)/,
        dateFormats: [
            "MMMM d, yyyy h:mm t",
            "MMMM d, yyyy "
        ],
        startDateFormatRegex: /([A-z]+ \d{2}, \d{4}) \s* ?(\d{1,2}:\d{2} ?(?:[A-Z]{2,})?)?/,
        endDateFormatRegex: /- ([A-z]+ \d{1,2}, \d{4})? ?\s* ?(\d{1,2}:\d{2} ?(?:[A-Z]{2,})?)?/
    }
}

function parsePdf(pdf, node, barcode) {
    const res = JsonLd.newEventReservation();
    const page = pdf.pages[barcode.location]

    for (let langCode of Object.keys(i18n)) {
        const lang = i18n[langCode]
        res.reservedTicket.ticketToken = 'qrCode:' + barcode.content;
        res.reservationNumber = barcode.content // QR Code content is shown on the ticket

        // Assuming the name is the first line of the ticket text.
        // Ticket issuers can pick from diffrent templates, or can create their own
        res.reservationFor.name = page.text.split("\n")[0]


        let [_ts, startDate, startTime] = lang.startDateFormatRegex.exec(page.text) ?? page.text.match(lang["Date"]) ?? []
        let [_te, endDate, endTime] = lang.endDateFormatRegex.exec(page.text) ?? []

        if (!_ts) {
            // This language doesn't match the ticket
            continue
        }

        res.reservationFor.startDate = JsonLd.toDateTime(
            startDate + " " + (startTime ?? ""), 
            lang.dateFormats,
            ["sk", "en"]
        )
        res.reservationFor.endDate = JsonLd.toDateTime(
            (endDate ?? startDate) + " " + (endTime ?? ""), 
            lang.dateFormats,
            ["sk", "en"]
        )

        res.underName.name = page.text.match(lang["Ticket Holder"])?.[1]
        res.reservationFor.location.name = page.text.match(lang["Venue name"])?.[1]
        res.reservedTicket.name = page.text.match(lang["Ticket Type"])?.[1]

        // Sometimes the price appears
        ExtractorEngine.extractPrice(page.text, res);
    }

    return res
}