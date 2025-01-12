/*
 *    SPDX-FileCopyrightText: 2025 Joshua Goins <josh@redstrate.com>
 *
 *    SPDX-License-Identifier: LGPL-2.0-or-later
 */

// Example: Wednesday, January 01, 2025 01:00 AM
const brightlineDateFormat = "dddd, MMMM d, yyyy h:mm ap";

// Example: Mar 10, 2024 05:15 PM
const brightlineTicketDateFormat = "MMM d, yyyy h:mm ap";

function collectChildren(element) {
    var children = new Array();
    var child = element.firstChild;
    while (!child.isNull) {
        children.push(child);
        child = child.nextSibling;
    }
    return children;
}

// Removes the extra whitespace in the time ("00:00         AM")
function cleanupTime(time) {
    var parts = time.match(/([0-9]{2}):([0-9]{2})\s*(PM|AM)/);
    return parts[1] + ":" + parts[2] + " " + parts[3];
}

function main(html) {
    let res = JsonLd.newTrainReservation();

    const ticketNumberElement = html.eval("//p[contains(@class, 'ticket-info')]")[0]
    if (!ticketNumberElement) {
        return;
    }

    const ticketNumber = ticketNumberElement.content.match(/Ticket\sNumber:\s([A-Z0-9]+)/);
    if (!ticketNumber) {
        return;
    }

    res.reservationNumber = ticketNumber[1];

    const passengerNameElement = html.eval("//p[contains(@class, 'passenger-name')]")[0]
    if (!passengerNameElement) {
        return;
    }

    res.underName.name = passengerNameElement.content;

    const tripDateElement = html.eval("//p[contains(@class, 'trip-date')]/a")[0]
    if (!tripDateElement) {
        return;
    }

    const tables = html.eval("//table[contains(., 'Departs')]/tbody");
    // Last element is the only valid one from this XPath
    const tripInfo = tables[tables.length - 1];

    // Text actually does have a space around it in the HTML
    const departsElement = tripInfo.eval("tr/td/p[text()=' Departs ']/preceding-sibling::h4")[0];
    if (!departsElement) {
        return;
    }
    res.reservationFor.departureStation.name = departsElement.content;

    const departureTimeElement = tripInfo.eval("tr/td/p[text()=' Departs ']/following-sibling::p/a")[0];
    if (!departureTimeElement) {
        return;
    }

    res.reservationFor.departureTime = JsonLd.toDateTime(tripDateElement.content + " " + cleanupTime(departureTimeElement.content), brightlineDateFormat, "en")

    const arrivesElement = tripInfo.eval("tr/td/p[text()=' Arrives ']/preceding-sibling::h4")[0];
    if (!arrivesElement) {
        return;
    }
    res.reservationFor.departureStation.name = arrivesElement.content;

    const arrivalTimeElement = tripInfo.eval("tr/td/p[text()=' Arrives ']/following-sibling::p/a")[0];
    if (!arrivalTimeElement) {
        return;
    }

    res.reservationFor.arrivalTime = JsonLd.toDateTime(tripDateElement.content + " " + cleanupTime(arrivalTimeElement.content), brightlineDateFormat, "en")

    return res;
}

function parsePdfBoardingPass(pdf, node, triggerNode) {
    let res = JsonLd.newTrainReservation();

    const page = pdf.pages[0];

    const departureTime = page.textInRect(0.2, 0.6, 0.4, 0.67);
    const arrivalTime = page.textInRect(0.6, 0.6, 0.9, 0.67);
    const departureDestination = page.textInRect(0.1, 0.7, 0.4, 0.75);
    const arrivalDestination = page.textInRect(0.6, 0.7, 0.9, 0.75);
    const ticketNumber = page.textInRect(0.67, 0.75, 0.9, 0.8);
    const tripDate = page.textInRect(0.3, 0.75, 0.5, 0.8);
    const name = page.textInRect(0.0, 0.1, 1.0, 0.15);

    res.reservationFor.departureTime = JsonLd.toDateTime(tripDate + " " + departureTime, brightlineTicketDateFormat, "en")
    res.reservationFor.arrivalTime = JsonLd.toDateTime(tripDate + " " + arrivalTime, brightlineTicketDateFormat, "en")
    res.reservationFor.departureStation.name = departureDestination;
    res.reservationFor.arrivalStation.name = arrivalDestination;
    res.reservationNumber = ticketNumber;
    res.underName.name = name;
    res.underName.name = name;
    res.reservedTicket.ticketToken = "qrCode:" + Barcode.decodeQR(page.images[0]);

    // TODO: Parse the coach and seat number
    // TODO: There's also station shortcodes like "ORL" and "MIA", but not sure if they're too useful yet

    return res;
}
