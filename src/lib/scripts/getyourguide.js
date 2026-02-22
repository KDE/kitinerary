// SPDX-FileCopyrightText: 2026 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: LGPL-2.0-or-later

function extractPdf(pdf, node) {
    let reservations = [];
    let i = 0;
    for (let page of pdf.pages) {
        const content = page.text;
        const lines = content.split('\n');

        const bookingNumber = content.match('Booking reference: (.*)');
        if (!bookingNumber) {
            break;
        }

        let res = JsonLd.newEventReservation();
        res.reservationNumber = bookingNumber[1];
        res.reservationFor.name = lines[2] + ' ' + lines[3];
        for (let i = 0, count = lines.length; i < count; i++) {
            const line = lines[i].trim();
            if (line.startsWith('')) {
                let match = line.match(", (.*) at ([0-9]{1,2}:[0-9]{2} ?(AM|PM)?)");
                res.reservationFor.startDate = JsonLd.toDateTime(match[1] + ' ' + match[2], ["MMMM dd, yyyy hh:mm AP", "MMMM dd, yyyy h:mm AP"], ['en']);
            }
        }

        const images = node.findChildNodes({ mimeType: "internal/qimage", scope: "Descendants" });
        const image = images[i * 2 + 1];
        if (image.childNodes && image.childNodes.length === 1 && image.childNodes[0].mimeType === "text/plain") {
            const barcode = image.childNodes[0].content;
            if (barcode) {
                res.reservedTicket.ticketToken = "qrCode:" + barcode;
            }
        }
        reservations.push(res);
        i++;
    }
    return reservations;
}
