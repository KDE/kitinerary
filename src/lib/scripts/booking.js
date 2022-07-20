/*
   SPDX-FileCopyrightText: 2018 Benjamin Port <benjamin.port@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

var regExMap = [];
regExMap['en_US'] = [];
regExMap['en_US']['bookingRef'] = /Booking number +([0-9]*)\s+/;
// 1: adress, 2: city, 3:postal code, 4: country, 5: phone
regExMap['en_US']['hotelInformation'] = / *(.+), (.+), (.+), (.+) -\s+Phone: (\+[0-9 ]*)\s+/;
regExMap['en_US']['hotelName'] = /\[checkmark\.png\] (.*) is expecting you on/;
regExMap['en_US']['arrivalDate'] = /Check-in *([A-z]+ [0-9]{1,2} [A-z]+ [0-9]+) \(f?r?o?m? ?([0-9]{1,2}:[0-9]{2})[^\)]*\)/;
regExMap['en_US']['departureDate'] = /Check-out *([1-z]+ [0-9]{1,2} [A-z]+ [0-9]+) \(.* ([0-9]{1,2}:[0-9]{2})\)/;
regExMap['en_US']['person'] = /Guest name +(.*) Edit guest name/;
regExMap['en_US']['dateFormat'] = "dddd d MMMM yyyy hh:mm";

regExMap['fr_FR'] = [];
regExMap['fr_FR']['bookingRef'] = /Numéro de réservation : ([0-9]*)\s+/;
// 1: hotel name, 2: adress, 3: city, 4:postal code, 5: country, 6: phone
regExMap['fr_FR']['hotelInformation'] = /(.+), (.+), (.+), (.+) -\s+Téléphone : (\+[0-9]*)\s+/;
regExMap['fr_FR']['hotelName'] = /L'établissement (.*) vous attend le/;
regExMap['fr_FR']['arrivalDate'] = /Arrivée  ([a-z]+ [0-9]{1,2} [a-zûé]+ [0-9]+) \(([0-9]{1,2}:[0-9]{2}) - ([0-9]{1,2}:[0-9]{2})\)/;
regExMap['fr_FR']['departureDate'] = /Départ  ([a-z]+ [0-9]{1,2} [a-zûé]+ [0-9]+) \([0-9]{1,2}:[0-9]{2} - ([0-9]{1,2}:[0-9]{2})\)/;
regExMap['fr_FR']['person'] = /Clients +(.*) Modifier le nom du client/;
regExMap['fr_FR']['dateFormat'] = "dddd d MMMM yyyy hh:mm";

regExMap['de_DE'] = [];
regExMap['de_DE']['bookingRef'] = /Buchungsnummer: ([0-9]*)\s+/;
// 1: hotel name, 2: adress, 3: city, 4:postal code, 5: country, 6: phone
regExMap['de_DE']['hotelInformation'] = /(.+), (.+), (.+), (.+) -\s+Telefon: (\+[0-9 \-]+)\n/;
regExMap['de_DE']['hotelName'] = /\[checkmark.png\] Die Unterkunft (.*)\s+erwartet Sie/;
regExMap['de_DE']['arrivalDate'] = /Anreise ([A-Z][a-z]+, [0-9]{1,2}\. \S+ [0-9]{4}) \(ab ([0-9]{1,2}:[0-9]{2})\)/;
regExMap['de_DE']['departureDate'] = /Abreise ([A-Z][a-z]+, [0-9]{1,2}\. \S+ [0-9]{4}) \(bis ([0-9]{1,2}:[0-9]{2})\)/;
regExMap['de_DE']['person'] = /Name des Gastes +(.*) Name des Gastes bearbeiten/;
regExMap['de_DE']['dateFormat'] = "dddd, d. MMMM yyyy hh:mm";

function main(text, node) {
    if (node.result.length > 0)
        return null; // this is just backup if we have no structured data
    var res = JsonLd.newLodgingReservation();

    for (var locale in regExMap) {
        var bookingRef = text.match(regExMap[locale]['bookingRef']);
        // If no booking reference go to the next locale
        if (!bookingRef)
            continue;
        res.reservationNumber = bookingRef[1];

        var hotelName = text.match(regExMap[locale]['hotelName']);
        if (!hotelName)
            return null;
        res.reservationFor.name = hotelName[1];

        var hotel = text.match(regExMap[locale]['hotelInformation']);
        if (!hotel)
            return null;

        res.reservationFor.address.streetAddress = hotel[1];
        res.reservationFor.address.postalCode = hotel[3];
        res.reservationFor.address.addressLocality = hotel[2];
        res.reservationFor.address.addressCountry = hotel[4];
        res.reservationFor.telephone = hotel[5];

        idx = hotel.index + hotel[0].length;

        var arrivalDate = text.substr(idx).match(regExMap[locale]['arrivalDate']);
        if (!arrivalDate)
            return null;

        res.checkinTime = JsonLd.toDateTime(arrivalDate[1] + " " + arrivalDate[2], regExMap[locale]['dateFormat'], locale);

        idx += arrivalDate.index + arrivalDate[0].length;

        var departureDate = text.substr(idx).match(regExMap[locale]['departureDate']);
        if (!departureDate)
            return null;
        res.checkoutTime = JsonLd.toDateTime(departureDate[1] + " " + departureDate[2], regExMap[locale]['dateFormat'], locale);
        idx += departureDate.index + departureDate[0].length;

        var name = text.substr(idx).match(regExMap[locale]['person']);
        if (!name)
            return null;
        res.underName.name = name[1];

        return res;
    }
}

function parseHtmlCommon(doc, node, res)
{
    const aElems = doc.eval('//a');
    for (aElem of aElems) {
        const href = aElem.attribute('href');
        if (href.startsWith('tel:')) {
            res.reservationFor.telephone = aElem.content;
        } else if (href.startsWith('mailto:')) {
            res.reservationFor.email = href.substr(7);
            // reservation id is the prefix in the mailto link, unlike other occurrences this seems most reliably present
            res.reservationNumber = href.match(/mailto:(\d+)-/)[1];
        } else if (aElem.attribute('universal') == 'true') {
            res.reservationFor.name = aElem.content;
        } else if (aElem.content.match(/modify/) && href.startsWith("https:")) {
            res.modifyReservationUrl = href;
        }
    }

    const times = doc.eval('//time');
    res.checkinTime = times[0].attribute("datetime");
    res.checkoutTime = times[1].attribute("datetime");

    const guest = doc.root.recursiveContent.match(/(?:Guest name|Nombre del huésped|Name des Gastes)[\n\s]+?(\S.*)\n/);
    if (guest) {
        res.underName.name = guest[1];
    }

    return res;
}

function parseHtml(doc, node)
{
    if (node.result.length > 0)
        return null; // this is just backup if we have no structured data
    var res = JsonLd.newLodgingReservation();
    var elem = doc.eval("//table[@class=\"mg_conf_hotel_preview\"]")[0];
    res.reservationFor.name = elem.eval("(.//b|.//strong)")[0].content;

    var fullAddr = elem.eval(".//tr")[1].recursiveContent;
    var addrRegex = /^(.*), (.*?), (.*?), ([^,]*?)\s*-?\s*$/;

    //HACK: Japanese addresses do not have the country set in Booking.com HTML
    // and have a different HTML structure
    if (!addrRegex.test(fullAddr)) {
        // The first two elements are the hotel name and the hotel name in
        // Japanese, skip to the third
        var addressElement = elem.eval("string(.//tr[3]//td)");
        // Booking.com addresses are always separated by "\n-\n"
        // We split and get the first part, which is the romanized address
        var fullAddr = addressElement.split("\n-\n")[0]
        // Replace double spaces from the extraction
        fullAddr = fullAddr.replace(/\s+/g, ' ');
        var addr = fullAddr.match(addrRegex);
        res.reservationFor.address.streetAddress = addr[4];
        res.reservationFor.address.addressLocality = addr[2];
        res.reservationFor.address.postalCode = addr[1];
        res.reservationFor.addressCountry = "Japan";
    } else {
        var addr = fullAddr.match(addrRegex);
        res.reservationFor.address.streetAddress = addr[1];
        res.reservationFor.address.addressLocality = addr[2];
        res.reservationFor.address.postalCode = addr[3];
        res.reservationFor.address.addressCountry = addr[4];

        if (fullAddr.match(/CANCELED$/)) {
            res.reservationStatus = "ReservationCancelled"
        }
    }
    res.reservationFor.telephone = elem.eval(".//*[@class=\"u-phone\"]")[0].content;
    return parseHtmlCommon(doc, node, res);
}

function parseHtmlAlternative(doc, node)
{
    if (node.result.length > 0)
        return null; // this is just backup if we have no structured data
    var res = JsonLd.newLodgingReservation();

    const addrElems = doc.eval('//address')[0].content.split(',\n');
    res.reservationFor.address.streetAddress = addrElems[0];
    res.reservationFor.address.addressLocality = addrElems[addrElems.length - 3];
    res.reservationFor.address.postalCode = addrElems[addrElems.length - 2];
    res.reservationFor.address.addressCountry = addrElems[addrElems.length - 1];
    return parseHtmlCommon(doc, node, res);
}
