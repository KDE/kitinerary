/*
   SPDX-FileCopyrightText: 2018 Benjamin Port <benjamin.port@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

var regExMap = [];
regExMap['en'] = {
    bookingRef: /(?:Booking number|Confirmation:) +([0-9]*)\s+/,
    // 1: adress, 2: city, 3:postal code, 4: country, 5: phone
    hotelInformation: / *(.+), (.+), (.+), (.+)(?: -|\n)\s+Phone:? (\+[0-9 ]*)\s+/,
    hotelName: [/(?:\[checkmark\.png\] |\.\d\n)(.*?)(?: is expecting you on|\n *\[)/, /\n\n\s*(\S.*\S)\n\n\s*Reservation details\n/],
    arrivalDate: /Check-in *([A-z]+,? [0-9]{1,2} [A-z]+ [0-9]+|[A-z]+, [A-z]+ \d{1,2}, \d{4}) \(f?r?o?m? ?([0-9]{1,2}:[0-9]{2})[^\)]*\)/,
    departureDate: /Check-out *([A-z]+,? [0-9]{1,2} [A-z]+ [0-9]+|[A-z]+, [A-z]+ \d{1,2}, \d{4}) \(.*?([0-9]{1,2}:[0-9]{2})\)/,
    person: /Guest name[\n\s]+(.*?)(?:\n| Edit guest name)/
}

regExMap['fr'] = {
    bookingRef: /Numéro de réservation : ([0-9]*)\s+/,
    // 1: hotel name, 2: adress, 3: city, 4:postal code, 5: country, 6: phone
    hotelInformation: /(.+), (.+), (.+), (.+) -\s+Téléphone : (\+[0-9]*)\s+/,
    hotelName: [/L'établissement (.*) vous attend le/],
    arrivalDate: /Arrivée  ([a-z]+ [0-9]{1,2} [a-zûé]+ [0-9]+) \(([0-9]{1,2}:[0-9]{2}) - ([0-9]{1,2}:[0-9]{2})\)/,
    departureDate: /Départ  ([a-z]+ [0-9]{1,2} [a-zûé]+ [0-9]+) \([0-9]{1,2}:[0-9]{2} - ([0-9]{1,2}:[0-9]{2})\)/,
    person: /Clients[\n\s]+(.*?)(?:\n| Modifier le nom du client)/
}

regExMap['de'] = {
    bookingRef: /(?:Buchungsnummer|Bestätigungsnummer): ([0-9]*)\s+/,
    // 1: hotel name, 2: adress, 3: city, 4:postal code, 5: country, 6: phone
    hotelInformation: /(?:Lage )?(.+), (.+), (.+), (.+) ?-?\s+Telefon:? (\+[0-9 \-]+)\n/,
    hotelName: [/\[checkmark.png\] Die Unterkunft (.*)\s+erwartet Sie/, /\n\n\s*(\S.*\S)\n\n\s*\[\S/],
    arrivalDate: /Anreise ([A-Z][a-z]+, [0-9]{1,2}\. \S+ [0-9]{4}) \((?:ab )?([0-9]{1,2}:[0-9]{2}).*\)/,
    departureDate: /Abreise ([A-Z][a-z]+, [0-9]{1,2}\. \S+ [0-9]{4}) \(bis ([0-9]{1,2}:[0-9]{2})\)/,
    person: /Name des Gastes[\n\s]+(.*?)(?:\n| Name des Gastes bearbeiten)/,
}

regExMap['da'] = {
    bookingRef: /Bekræftelsesnummer: ([0-9]*)\s+/,
    hotelInformation: /Beliggenhed[\s\n]+(.+), (.+)(.*), (.+)[\s\n]+Telefon (\+[0-9 \-]+)\n/,
    hotelName: [/(.*) forventer at se/],
    arrivalDate: /Indtjekning .* (\d{1,2}. \S+ \d{4}) \(.*(\d\d\.\d\d)\)/,
    departureDate: /Udtjekning .* (\d{1,2}. \S+ \d{4}) \(.*(\d\d\.\d\d)\)/,
    person: /Gæster[\s\n]+(.*)\n/
}

regExMap['es'] = {
    bookingRef: /Confirmación: ([0-9]*)\s+/,
    hotelInformation: /(\S.+), (.+),[\n ]([^,]+),[\n ]([^,]+)\n\n\s+Teléfono (\+[0-9- ]*)\s+/,
    hotelName: [/El (\S.*\S) te espera/],
    arrivalDate: /Entrada (\S+, [0-9]{1,2} de \S+ de [0-9]{4}) \(.*?([0-9]{1,2}:[0-9]{2}).*\)/,
    departureDate: /Salida (\S+, [0-9]{1,2} de \S+ de [0-9]{4}) \(.* ([0-9]{1,2}:[0-9]{2})\)/,
    person: /Nombre del huésped[\n\s]+(.*?)\n/
}

const timeFormats = [
    "dddd d MMMM yyyy hh:mm",
    "dddd, d MMMM yyyy hh:mm",
    "dddd, d. MMMM yyyy hh:mm",
    "dddd, MMMM d, yyyy hh:mm",
    "dddd, dd 'de' MMMM 'de' yyyy hh:mm",
    "d. MMMM yyyy hh.mm"
];


function main(text, node) {
    if (node.result.length > 0)
        return null; // this is just backup if we have no structured data
    var res = JsonLd.newLodgingReservation();

    for (var locale in regExMap) {
        const bookingRef = text.match(regExMap[locale]['bookingRef']);
        // If no booking reference go to the next locale
        if (!bookingRef || !regExMap[locale]['bookingRef'])
            continue;
        res.reservationNumber = bookingRef[1];

        let hotelName = undefined;
        for (const nameRx of regExMap[locale]['hotelName']) {
            hotelName = text.match(nameRx);
            if (hotelName)
                break;
        }
        res.reservationFor.name = hotelName[1];

        const hotel = text.match(regExMap[locale]['hotelInformation']);
        res.reservationFor.address.streetAddress = hotel[1];
        res.reservationFor.address.postalCode = hotel[3];
        res.reservationFor.address.addressLocality = hotel[2];
        res.reservationFor.address.addressCountry = hotel[4];
        res.reservationFor.telephone = hotel[5];

        const arrivalDate = text.match(regExMap[locale]['arrivalDate']);
        res.checkinTime = JsonLd.toDateTime(arrivalDate[1] + " " + arrivalDate[2], timeFormats, locale);

        const departureDate = text.match(regExMap[locale]['departureDate']);
        res.checkoutTime = JsonLd.toDateTime(departureDate[1] + " " + departureDate[2], timeFormats, locale);

        const name = text.match(regExMap[locale]['person']);
        res.underName.name = name[1];

        return res;
    }
}

function parseDateTimeAttribute(attr) {
    // valid ISO format (when time is present)
    if (attr.match(/^\d{4}-\d\d-\d\dT\d\d:\d\d/)) {
        // chop of UTC offset if present, that is based on time of booking, not time of travel
        return attr.substr(0, 19);
    }
    return JsonLd.toDateTime(attr.substr(0, 10), "yyyy-MM-dd", "en");
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
        } else if ((aElem.attribute('universal') == 'true' || href.match(/booking.com\/hotel\//)) && aElem.recursiveContent !== "" && !res.reservationFor.name) {
            res.reservationFor.name = aElem.recursiveContent;
        } else if (!res.modifyReservationUrl && href.startsWith("https:") && (href.match(/pbsource=email_change;/) || href.match(/pbsource=conf_email_modify;/))) {
            res.modifyReservationUrl = href;
        }
    }

    const times = doc.eval('//time');
    res.checkinTime = parseDateTimeAttribute(times[0].attribute("datetime"));
    res.checkoutTime = parseDateTimeAttribute(times[1].attribute("datetime"));

    const text = doc.root.recursiveContent;
    for (let locale in regExMap) {
         const name = text.match(regExMap[locale]['person']);
         if (name) {
             res.underName.name = name[1];
             break;
         }
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
        res.reservationFor.address.addressCountry = addr[4].split('\n')[0];

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
