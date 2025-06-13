/*
 * SPDX-FileCopyrightText: 2025 Jonas Junker <jonassimonjunker@proton.me>
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

function main(pdf, node) {

    // create new booking reference
    var res = JsonLd.newLodgingReservation();

    // extract all text from the pdf
    const text = pdf.text;
    // also extract the pageXOffset
    const page = pdf.pages[0];

    // get the reservation number
    var bookingRef = text.match(/Your reference number is ([0-9-]+)./)[1];
    res.reservationNumber = bookingRef;

    // get the checkin time
    checkinTime = text.match(/Check in:\s+(\S+)\s+to/);

    // get the checkout time
    checkoutTime = text.match(/Check out:\s\S*\s?[by|latest]\s*(\S+)/)

    // get the arrival and departure dates
    var regexTable = /(\d{2}\/\d{2}\/\d{4})\s+(\d{1,3})/g;

    // loop through the list of dates of reservations and determine the date of
    // the checkout
    const timeformats = ["d/M/yyyy h:mmAP", "d/M/yyyy hAP"]
    while ((match = regexTable.exec(text)) !== null) {

        // set arrival date for first match
        if (res.checkinTime === undefined) {
            res.checkinTime = JsonLd.toDateTime(match[1] + " "  + checkinTime[1], timeformats, "en");
        }

        // checkoutTime is just the date of the current loop plus the numer of nights (match[4] of this row
        res.checkoutTime = JsonLd.toDateTime(match[1] + " " + checkoutTime[1], timeformats, "en");
        res.checkoutTime.setDate(res.checkoutTime.getDate() + parseInt(match[2], 10))
    }

    // name of person reserving
    var namePerson = text.match(/Dear ([A-Z ]+),/i)[1];
    res.underName.name = namePerson

    // name and address of hostel
    var hostel = text.match(/\s+(\w+\s.*)\s+Tel:\s+(\+?44? ?\(\d+\)?\s+[0-9 ]+)\s+Address:\s+([^,]+),\s*([^,]+)(?:,\s*([^,]+))?\s+(\S{2,4} \S{3})\s+Email:\s+(\S+@\S+)\s+([0-9 ]+)\s+Directions/)

    // name of hostel
    res.reservationFor.name = hostel[1]

    // telephone number of hostel. May be split in 2 parts
    var telephone = hostel[2];

    if (hostel[8] !== undefined) {
        telephone += " " + hostel[8];
    }
    res.reservationFor.telephone = telephone;

    // e-mail adress of hostel
    res.reservationFor.email = hostel[7];

    // adress of hostel
    res.reservationFor.address.streetAddress = hostel[3];
    res.reservationFor.address.addressLocality = hostel[4];
    if (hostel[5] !== undefined ) {
            res.reservationFor.address.addressRegion = hostel[5];
    }
    res.reservationFor.address.postalCode = hostel[6];

    // geo coordinates of hostel
    const links = page.linksInRect(0, 0, 1, 1);
    res.reservationFor.geo = JsonLd.toGeoCoordinates(links[1].url);

    return res
}


