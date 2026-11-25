// SPDX-FileCopyrightText: 2026 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: LGPL-2.0-or-later

function parseQrTicket(html)
{
    if (!html.eval("//input[@name='_PageID'][@value='RSWP230P107']").length) {
        return;
    }

    const journey = html.eval("//section[@class='main-contents']")[0];
    if (!journey) {
        return;
    }
    const reservationNumber = journey.eval("h2")[0]?.content.match(/Reservation number\s+(\d+)/);
    const date = journey.eval(".//table[@class='date']")[0]?.recursiveContent.match(/([A-Za-z]{3})\. (\d{1,2}), (\d{4})/);
    const times = journey.eval(".//table[@class='departure']")[0]?.recursiveContent.replace(/\s/g, '').match(/(\d{1,2}:\d{2})→(\d{1,2}:\d{2})/);
    const departure = journey.eval(".//table[@class='name']//td[@class='a']")[0];
    const arrival = journey.eval(".//table[@class='name']//td[@class='c']")[0];
    if (!reservationNumber || !date || !times || !departure || !arrival) {
        return;
    }

    let res = JsonLd.newTrainReservation();
    res.reservationNumber = reservationNumber[1];
    const dateString = date[1] + ' ' + date[2] + ' ' + date[3] + ' ';
    res.reservationFor.departureTime = JsonLd.toDateTime(dateString + times[1], 'MMM d yyyy h:mm', 'en');
    res.reservationFor.arrivalTime = JsonLd.toDateTime(dateString + times[2], 'MMM d yyyy h:mm', 'en');
    res.reservationFor.departureStation.name = departure.recursiveContent;
    res.reservationFor.departureStation.address.addressCountry = 'JP';
    res.reservationFor.arrivalStation.name = arrival.recursiveContent;
    res.reservationFor.arrivalStation.address.addressCountry = 'JP';
    const train = journey.eval(".//table[not(@class)]")[0]?.recursiveContent.match(/([A-Z]+)\s*(\d+)/);
    if (train) {
        res.reservationFor.trainName = train[1];
        res.reservationFor.trainNumber = train[2];
    }

    const seatElement = html.eval("//div[@class='qr_section']//td[@class='seat']")[0];
    const seat = seatElement?.content.match(/Car\.\s*(\d+)\s+Seat\.\s*(\S+)/);
    if (seat) {
        res.reservedTicket.ticketedSeat.seatSection = seat[1];
        res.reservedTicket.ticketedSeat.seatNumber = seat[2];
    }

    const labelElement = html.eval("//div[@class='qr_section']//div[@class='qr_label']")[0];
    const label = labelElement?.recursiveContent.replace(/\s/g, '');
    if (label && /^[0-9A-F]{32}$/.test(label)) {
        // Same as the code embedded in the QR code
        res.reservedTicket.ticketToken = 'qrCode:' + '0'.repeat(68) + label + '0'.repeat(8);
    }
    return res;
}
