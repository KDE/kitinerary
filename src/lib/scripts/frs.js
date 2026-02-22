// SPDX-FileCopyrightText: 2026 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: LGPL-2.0-or-later

function extractPdf(pdf, node, barcode) {
    const content = pdf.text;
    const lines = content.split('\n');
    const reservations = [];
    let departureStation;
    let departureTime;
    let arrivalStation;
    let bookingNumber;

    for (let i = 0, count= lines.length; i < count; i++) {
        const line = lines[i].trim();
        if (line.startsWith('BOOKING NUMBER')) {
            bookingNumber = lines[i+1];
        }
        if (line.startsWith('DEPARTURE')) {
            const match = line.match('-(.*) >> (.*)');
            departureStation = match[1].trim();
            arrivalStation = match[2].trim();
            departureTime = JsonLd.toDateTime(lines[i + 1].trim().split('     ')[0], ["dd.MM.yyyy hh:mm"], ["en"]);
            break;
        }
    }

    let barcodeText = null;

    const images = node.findChildNodes({ mimeType: "internal/qimage", scope: "Descendants" });

    for (let image of images) {
        if (image.childNodes && image.childNodes.length === 1 && image.childNodes[0].mimeType === "text/plain") {
            const barcode = image.childNodes[0].content;
            if (barcode) {                barcodeText = "qrCode:" + barcode;
                break;
            }
        }
    }

    // hardcode because FRS only operates 3 ports
    const ports = {
        'Algeciras': [36.131294, -5.438265, 'Puerto de Algeciras', 11201, 'Algeciras'],
        'Tangier Med': [35.876633, -5.5118833, 'Port Tanger Med', null, 'Tanger'],
        'Ceuta': [35.894489, -5.320913, 'Ferry Ceuta Algeciras', 51002, 'Ceuta'], // guess
    };

    for (let i = 0, count = lines.length; i < count; i++) {
        const line = lines[i].trim();
        if (line.startsWith('Passenger')) {
            i++;
            while (lines[i].trim().length > 0) {
                const res = JsonLd.newBoatReservation();
                res.reservationNumber = bookingNumber;
                res.reservedTicket.ticketToken = barcodeText;
                const info = lines[i].split('       ').filter(text => text.trim().length > 0);
                res.underName.name = info[0];
                res.reservationFor.departureTime = departureTime;
                res.reservationFor.departureBoatTerminal.name = ports[departureStation][2];
                res.reservationFor.departureBoatTerminal.geo.latitude = ports[departureStation][0];
                res.reservationFor.departureBoatTerminal.geo.longitude = ports[departureStation][1];
                res.reservationFor.departureBoatTerminal.address.postalCode = ports[departureStation][3];
                res.reservationFor.departureBoatTerminal.address.addressLocality = ports[departureStation][4];

                res.reservationFor.arrivalBoatTerminal.name = ports[arrivalStation][2];
                res.reservationFor.arrivalBoatTerminal.geo.latitude = ports[arrivalStation][0];
                res.reservationFor.arrivalBoatTerminal.geo.longitude = ports[arrivalStation][1];
                res.reservationFor.arrivalBoatTerminal.address.postalCode = ports[arrivalStation][3];
                res.reservationFor.arrivalBoatTerminal.address.addressLocality = ports[arrivalStation][4];

                ExtractorEngine.extractPrice(info[5].replace(' ', ''), res);
                i += 4;
                reservations.push(res);
            }
        }
    }
    return reservations;
}
