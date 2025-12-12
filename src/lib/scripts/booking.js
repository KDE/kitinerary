/*
   SPDX-FileCopyrightText: 2018 Benjamin Port <benjamin.port@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

var regExMap = [];
regExMap['en'] = {
    bookingRef: /(?:Booking number|Confirmation:) +([0-9]*)\s+/,
    hotelInformation: /(?:Location )?(\S[^\n]*(?:,[^\n,]*\n?[^\n,]*){1,3})(?: -|\n)\s*Phone:? ?(?:[\s]+?)(\+[0-9 ]*)\s+/,
    hotelName: [/(?:\[checkmark\.png\] |\.\d\n)(.*?)(?: is\s+expecting you on|\n *\[)/, /\n\n\s*(?:You'll pay when you stay at )?(\S.*\S)\n\n\s*Reservation details\n/, /Please inform (.*) of your expected/],
    arrivalDate: /Check-in *?\s+? *?([A-z]+,? [0-9]{1,2} [A-z]+ [0-9]+|[A-z]+, [A-z]+ \d{1,2}, \d{4}) \(f?r?o?m? ?([0-9]{1,2}:[0-9]{2}(?: [AP]M)?)[^\)]*\)/,
    departureDate: /Check-out *?\s+? *?([A-z]+,? [0-9]{1,2} [A-z]+ [0-9]+|[A-z]+, [A-z]+ \d{1,2}, \d{4}) \(.*?(?:- )?([0-9]{1,2}:[0-9]{2}(?: [AP]M)?)\)/,
    person: /Guest name[\n\s]+(.*?)(?:\n| Edit guest name)/
}

regExMap['fr'] = {
    bookingRef: /Numéro de réservation : ([0-9]*)\s+/,
    hotelInformation: /(\S[^,\n]*(?:,[^\n,]*\n?[^\n,]*){2,3}) -\s+Téléphone : (\+[0-9]*)\s+/,
    hotelName: [/L'établissement (.*) vous attend le/],
    arrivalDate: /Arrivée  ([a-z]+ [0-9]{1,2} [a-zûé]+ [0-9]+) \(([0-9]{1,2}:[0-9]{2}) - ([0-9]{1,2}:[0-9]{2})\)/,
    departureDate: /Départ  ([a-z]+ [0-9]{1,2} [a-zûé]+ [0-9]+) \([0-9]{1,2}:[0-9]{2} - ([0-9]{1,2}:[0-9]{2})\)/,
    person: /Clients[\n\s]+(.*?)(?:\n| Modifier le nom du client)/
}

regExMap['de'] = {
    bookingRef: /(?:Buchungsnummer|Bestätigungsnummer): ([0-9]*)\s+/,
    hotelInformation: /(?:Lage )?(\S[^\n]*(?:,[^\n,]*?\n?[^\n,]*?){2,3})[\n\s]?-?\s+(?:Reiseroute.*\n)?\s*(?:Telefon:? (\+[0-9 \-]+)|Kontakt.*|E-Mail an.*)\n/,
    hotelName: [/Die Unterkunft (.*)\s+erwartet Sie/, /\n\n\s*([^\s\[].*\S)\n\n\s*\[\S/],
    arrivalDate: /Anreise +([A-Z][a-z]+, [0-9]{1,2}\. \S+ [0-9]{4}) \((?:ab )?([0-9]{1,2}:[0-9]{2}).*\)/,
    departureDate: /Abreise +([A-Z][a-z]+, [0-9]{1,2}\. \S+ [0-9]{4}) \(.*?([0-9]{1,2}:[0-9]{2})\)/,
    person: /Name des Gastes[\n\s]+(.*?)(?:\n| Name des Gastes bearbeiten)/,
}

regExMap['da'] = {
    bookingRef: /Bekræftelsesnummer: ([0-9]*)\s+/,
    hotelInformation: /Beliggenhed[\s\n]+(\S[^,\n]*(?:,[^\n,]*\n?[^\n,]*){2,3})[\s\n]+Telefon (\+[0-9 \-]+)\n/,
    hotelName: [/(.*) forventer at se/],
    arrivalDate: /Indtjekning .* (\d{1,2}. \S+ \d{4}) \(.*(\d\d\.\d\d)\)/,
    departureDate: /Udtjekning .* (\d{1,2}. \S+ \d{4}) \(.*(\d\d\.\d\d)\)/,
    person: /Gæster[\s\n]+(.*)\n/
}

regExMap['es'] = {
    bookingRef: /(?:Confirmación: ([0-9]*)\s+|está confirmada)/,
    hotelInformation: /(?:Ubicación |Modificar tu reserva\n.*\n)?(\S[^\n]*(?:,[^\n,]*\n?[^\n,]*){2,})[\n\s]+Teléfono:? (\+[0-9- ]*)\s+/,
    hotelName: [/El (\S.*\S) te espera/, /reserva en (\S.*\S) está confirmada/],
    arrivalDate: /Entrada +(\S+, [0-9]{1,2} de \S+ de [0-9]{4}) \(.*?([0-9]{1,2}:[0-9]{2}).*\)/,
    departureDate: /Salida +(\S+, [0-9]{1,2} de \S+ de [0-9]{4}) \(.* ([0-9]{1,2}:[0-9]{2})\)/,
    person: /Nombre del huésped[\n\s]+(.*?)(?: Editar .*)?\n/
}

regExMap['it'] = {
    bookingRef: /(?:Conferma:) +([0-9]*)\s+/,
    hotelInformation: /(?:Location )?(\S[^\n]*(?:,[^\n,]*\n?[^\n,]*){1,3})(?: -|\n)\s*Telefono:? ?(?:[\s]+?)(\+[0-9 ]*)\s+/,
    hotelName: [/(?:\[checkmark\.png\] |\.\d\n)(.*?)(?: ti aspetta|\n *\[)/, /\n\n\s*(?:You'll pay when you stay at )?(\S.*\S)\n\n\s*Reservation details\n/, /Please inform (.*) of your expected/],
    arrivalDate: /Arrivo\s+(\S+ [0-9]{1,2} [A-z]+ [0-9]{4}) \(.*(\d{2}:\d{2})\)/,
    departureDate: /Partenza\s+(\S+ [0-9]{1,2} [A-z]+ [0-9]+|[A-z]+, [A-z]+ \d{1,2}, \d{4}) \(.*?(?: - )?(\d{2}:\d{2})\)/,
    person: /Nome dell'ospite[\n\s]+(.*?)\n/
}

const timeFormats = [
    "dddd d MMMM yyyy hh:mm",
    "dddd, d MMMM yyyy hh:mm",
    "dddd, d. MMMM yyyy hh:mm",
    "dddd, MMMM d, yyyy hh:mm",
    "dddd, MMMM d, yyyy hh:mm A",
    "dddd, MMMM d, yyyy h:mm A",
    "dddd, dd 'de' MMMM 'de' yyyy hh:mm",
    "dddd, d 'de' MMMM 'de' yyyy HH:mm",
    "d. MMMM yyyy hh.mm",
    "dddd dd MMMM yyyy HH:mm",
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
        const addr = hotel[1].split(/, */);
        res.reservationFor.address.streetAddress = addr.slice(0, addr.length >= 4 ? addr.length - 3 : addr.length - 2).join(", ");
        res.reservationFor.address.postalCode = addr.length >= 4 ? addr[addr.length - 2] : ""
        res.reservationFor.address.addressLocality = addr.length >= 4 ? addr[addr.length - 3] : addr[addr.length - 2];
        res.reservationFor.address.addressCountry = addr[addr.length - 1]
        res.reservationFor.telephone = hotel[2];

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
    if (node.result.length > 0 && node.result[0].url != undefined)
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

function parsePdf(pdf, node)
{
    var pdfText = pdf.text;

    var res = JsonLd.newLodgingReservation();

    var num = pdfText.match(/CONFIRMATION NUMBER: (.+)/)[1];
    res.reservationNumber = num.split(".").join("");

    var info = pdfText.match(/CHECK-OUT\s+ROOMS\s+NIGHTS\n\s+(.+)\n(.+)\n\s+(\d+)\s+(\d+)\s+\d+\s*\/\d+\n\s+Address:\s+(.+)\s+([A-Z]+)\s+([A-Z]+)\n\s+(\d+)\s+(.+)\n\s+.*\n\s+Phone: (.+)\n\s+from (\d\d:\d\d)\s+until (\d\d:\d\d)\n\s+GPS coordinates: (.+)/);

    res.reservationFor.name = info[1] + info[2];
    res.reservationFor.address.streetAddress = info[5];
    res.reservationFor.address.postalCode = info[8];
    res.reservationFor.address.addressLocality = info[9];
    res.reservationFor.telephone = info[10];

    var coords = parseCoordinates(info[13]);
    res.reservationFor.geo.latitude = coords[0];
    res.reservationFor.geo.longitude = coords[1];

    var checkin = info[3] + " " + info[6] + " " + info[11];
    var checkout = info[4] + " " + info[7] + " " + info[12];
    res.checkinTime = JsonLd.toDateTime(checkin, "d MMMM hh:mm", 'en');
    res.checkoutTime = JsonLd.toDateTime(checkout, "d MMMM hh:mm", 'en');

    return res;
}

function parseCoordinates(input) {
    const regex = /([NS])\s*(\d{1,3})[°º]?\s*(\d+(?:\.\d+)?)[,\s]+([EW])\s*(\d{1,3})[°º]?\s*(\d+(?:\.\d+)?)/i;
    const match = input.match(regex);

    const [, latHem, latDeg, latMin, lonHem, lonDeg, lonMin] = match;

    const latitude = (parseInt(latDeg, 10) + parseFloat(latMin) / 60) * (latHem.toUpperCase() === 'S' ? -1 : 1);
    const longitude = (parseInt(lonDeg, 10) + parseFloat(lonMin) / 60) * (lonHem.toUpperCase() === 'W' ? -1 : 1);

    return [latitude, longitude];
}
