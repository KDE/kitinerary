/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

function parseConfirmation(html, node) {
    let res = node.result.length === 1 ? node.result[0] : JsonLd.newLodgingReservation();

    const content = html.root.recursiveContent;
    res.reservationNumber = content.match(/(?:Reservation number|Reservation n°|Reservierung Nr.|Buchungsnummer):?\s*([0-9A-Z]+)/)[1];

    let dt = content.match(/(\d{2}[|/.]\d{2}[|/.]\d{4}).*(\d{2}[|/.]\d{2}[|/.]\d{4})/);
    if (!dt) // US format
        dt = content.match(/([A-Z][a-z]{2}\. \d{2}, \d{4}).*([A-Z][a-z]{2}\. \d{2}, \d{4})/);
    if (!dt)
        dt = content.match(/(\d{2} [A-Z][a-z]{2} \d{4}).*(\d{2} [A-Z][a-z]{2} \d{4})/);
    const times = content.match(/(?:Check in Policy|Anreisezeit):?\n.*(\d\d:\d\d).*\n(?:Check out Policy|Abreisezeit):?\n.*(\d\d:\d\d)/);
    const formats = [
        "dd.MM.yyyyHH:mm", "dd.MM.yyyy",
        "dd/MM/yyyyHH:mm", "dd/MM/yyyy",
        "MMM. dd, yyyyHH:mm", "MMM. dd, yyyy",
        "dd MMM yyyyHH:mm", "dd MMM yyyy"
    ];
    res.checkinTime = JsonLd.toDateTime(dt[1] + (times ? times[1] : ""), formats, "en");
    res.checkoutTime = JsonLd.toDateTime(dt[2] + (times ? times[2] : ""), formats, "en");

    let hotelNode = html.eval('//table//table//table[@class="table-full"]');
    if (!hotelNode || hotelNode.length === 0)
        hotelNode = html.eval('//table//table//table//table//tr/td/font/a/../../../..');
    hotelContent = hotelNode[0].recursiveContent;
    hotelContent = hotelContent.replace(/\s+\n/, "\n");

    var hotel = hotelContent.match(/^(.*)\n+(.*\n.*)\n(?:.|\n)*Tel\s*:\s*([\d \/\+\(\)]+)\n(.+@.+?)[\s\n]/);
    if (hotel) {
        res.reservationFor.telephone = hotel[3];
        res.reservationFor.email = hotel[4];
    } else {
        // no contact information present
        hotel = hotelContent.match(/^(.*)\n+(.*\n.*)\n(?:.|\n)*/);
    }

    if (hotel) {
        res.reservationFor.name = hotel[1];

        const addr = hotel[2].match(/(.*)(?: - |\n)(.*) - (.*)/);
        res.reservationFor.address.streetAddress = addr[1];
        res.reservationFor.address.addressLocality = addr[2];
        res.reservationFor.address.addressCountry = addr[3];
    }

    // this is in an HTML comment...
    if (html.rawData.match(/TARS - AH - Client - Cancellation - Email/)) {
        res.reservationStatus = "ReservationCancelled"
    }

    ExtractorEngine.extractPrice(content, res);
    const price = content.match(/(?:Total|Gesamt).*\n(\s*\n)?.*\d.*\n/);
    if (price && !res.totalPrice)
        ExtractorEngine.extractPrice(price[0], res);
    return res;
}
