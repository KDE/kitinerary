/*
   SPDX-FileCopyrightText: 2021 Nicolas Fella <nicolas.fella@gmx.de>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

function main(content) {

    const resId = content.match(/Buchungsnummer:\n?\s*(.+)/)[1]

    const arrivalDate = content.match(/Anreisetag:\n? *(.+)/)[1]
    const departureDate = content.match(/Abreisetag:\n? *(.+)/)[1]

    const guestName = content.match(/Gastname:\n? *(?:Herr|Frau)? ?(.+)/)[1]

    const addressBlock = content.match(/Ihr\n(.*)\n(.*)\n(.*)/)

    const hotelName = addressBlock[1]

    const street = addressBlock[2]

    const cityLine = addressBlock[3].match(/([0-9]+) (.*)/)

    const address = JsonLd.newObject("PostalAddress")
    address.addressCountry = "DE"
    address.addressLocality = cityLine[2]
    address.postalCode = cityLine[1]
    address.streetAddress = street

    const telephone = content.match(/Tel.: (.*)/)[1]
    const email = content.match(/E-Mail: +(.*)/)[1]

    const price = content.match(/Gesamtpreis:\n? *(.*) EUR/)[1].replace(',', '.')

    const numberAdults = content.match(/Anzahl der Erwachsene[rn]:\n? *([0-9]+)/)[1]

    const numberChildren = content.match(/Anzahl der Kinder:\n? *([0-9]+)/);

    var res = JsonLd.newLodgingReservation()

    res.reservationFor.name = addressBlock[1]

    res.reservationNumber = resId
    res.checkinTime = JsonLd.toDateTime(arrivalDate, "dd.MM.yyyy", "de")
    res.checkoutTime = JsonLd.toDateTime(departureDate, "dd.MM.yyyy", "de")

    res.reservationFor.telephone = telephone
    res.reservationFor.email = email

    res.reservationFor.address = address

    res.underName.name = guestName

    res.totalPrice = price
    res.priceCurrency = "EUR"

    res.reservationFor.numAdults = numberAdults
    res.reservationFor.numChildren = numberChildren ? numberChildren[1] : undefined;

    return res
}

function parsePkPass(pass)
{
    var res = JsonLd.newLodgingReservation();
    res.reservationFor.name = pass.organizationName;
    res.checkinTime = JsonLd.toDateTime(pass.field['BGcheckin'].value.substr(0, 18), 'dd.MM.yyyy - hh:mm', 'de');
    res.checkoutTime = JsonLd.toDateTime(pass.field['BGcheckout'].value.substr(0, 18), 'dd.MM.yyyy - hh:mm', 'de');
    res.reservationFor.geo.latitude = pass.locations[0].latitude;
    res.reservationFor.geo.longitude = pass.locations[0].longitude;
    res.reservationFor.address.streetAddress = pass.field['address'].value;
    res.reservationFor.telephone = pass.field['phone'].value;
    res.reservationNumber = pass.field['bookId'].value;
    res.underName.name = pass.field['guestname'].value;
    return res;
}

function parsePdf(pdf, node, triggerNode)
{
    const page = pdf.pages[triggerNode.location];
    let res = JsonLd.newLodgingReservation();
    res.reservationNumber = page.text.match(/.*: ([A-Z0-9-]+)/)[1];
    const leftCol = page.textInRect(0.0, 0.2, 0.5, 1.0);
    const dates = leftCol.match(/(\d\d\.\d\d.\d{4}).*(\d\d\.\d\d.\d{4})/);
    const times = leftCol.match(/(\d\d\:\d\d).*(\d\d\:\d\d)/);
    res.checkinTime = JsonLd.toDateTime(dates[1] + ' ' + times[1], 'dd.MM.yyyy hh:mm', 'de');
    res.checkoutTime = JsonLd.toDateTime(dates[2] + ' ' + times[2], 'dd.MM.yyyy hh:mm', 'de');
    const rightCol = page.textInRect(0.5, 0.2, 1.0, 1.0);
    const addr = rightCol.match(/.*\n(.*)\n(.*)\n(.*)/);
    res.reservationFor.name = addr[1];
    res.reservationFor.address.streetAddress = addr[2];
    res.reservationFor.address.addressLocality = addr[3];
    res.reservationFor.telephone = page.links[0].url.substr(4);
    res.reservationFor.email = page.links[1].url.substr(7);
    res.reservationFor.geo = JsonLd.toGeoCoordinates(page.links[2].url);
    res.modifyReservationUrl = page.links[3].url;
    return res;
}
