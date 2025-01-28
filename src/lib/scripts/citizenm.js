/*
 *   SPDX-FileCopyrightText: 2025 Joshua Goins <josh@redstrate.com>
 *   SPDX-License-Identifier: LGPL-2.0-or-later
 */

// Example: Wednesday, 01 January 2025 02:00 PM
const citizenMDateFormat = "dddd, d MMMM yyyy h:mm ap";

// Grab information from a central nested table in the middle of the HTML doc
function extractTableInformation(html, text) {
    const element = html.eval("//th[table[tr[td[contains(@class, 'font_16') and contains(., \'" + text + "\')]]]]/following-sibling::th/table/tr/td[1]")[0]
    if (!element)
        return null;

    return element.recursiveContent;
}

// Process text like "after 2.00 PM" and "before 11:00 AM" (yes, they mix and match . and : in the same email) and returns a time like "2:00 PM"
function extractRelativeTime(text) {
    const match = text.match(/(after|before)\s([\d])+.([\d]+)\s(AM|PM)/);
    return match[2] + ":" + match[3] + " " + match[4];
}

function extractHtml(html) {
    const text = html.root.recursiveContent;
    let res = JsonLd.newLodgingReservation();

    // The <td> contains spaces, so we can't use text() but use contains()
    var confirmationNumberElement = html.eval("//tr[td[contains(., 'booking code')]]/following-sibling::tr/td[1]")[0]
    if (!confirmationNumberElement)
        return {};

    res.reservationNumber = confirmationNumberElement.content;
    res.reservationFor.name = extractTableInformation(html, "booker");

    // "<Street Address>, <City>, <State> <Postal Code>, <Country>"
    const addr = extractTableInformation(html, "hotel address").match(/ *(.+), (.+), (.+) (.+), (.+)/);
    res.reservationFor.address.streetAddress = addr[1];
    res.reservationFor.address.addressLocality = addr[2];
    res.reservationFor.address.addressRegion = addr[3];
    res.reservationFor.address.postalCode = addr[4];
    res.reservationFor.address.addressCountry = addr[5];

    // First element is the date, second is the time (but not yet processed)
    const checkIn = extractTableInformation(html, "check-in").split("\n");
    const checkOut = extractTableInformation(html, "check-out").split("\n");

    res.checkinTime = JsonLd.toDateTime(checkIn[0] + " " + extractRelativeTime(checkIn[1]), citizenMDateFormat, "en");
    res.checkoutTime = JsonLd.toDateTime(checkOut[0] + " " + extractRelativeTime(checkOut[1]), citizenMDateFormat, "en");

    return res;
}
