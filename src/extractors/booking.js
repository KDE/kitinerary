/*
   Copyright (c) 2018 Benjamin Port <benjamin.port@kde.org>

   This library is free software; you can redistribute it and/or modify it
   under the terms of the GNU Library General Public License as published by
   the Free Software Foundation; either version 2 of the License, or (at your
   option) any later version.

   This library is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
   FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
   License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to the
   Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
   02110-1301, USA.
*/

var regExMap = [];
regExMap['en_US'] = [];
regExMap['en_US']['bookingRef'] = /Booking number +([0-9]*)\s+/;
// 1: adress, 2: city, 3:postal code, 4: country, 5: phone
regExMap['en_US']['hotelInformation'] = / *(.+), (.+), (.+), (.+) -\s+Phone: (\+[0-9]*)\s+/;
regExMap['en_US']['hotelName'] = /\[checkmark\.png\] (.*) is expecting you on/;
regExMap['en_US']['arrivalDate'] = /Check-in *([A-z]+ [0-9]{1,2} [A-z]+ [0-9]+) \(([0-9]{1,2}:[0-9]{2}) - ([0-9]{1,2}:[0-9]{2})\)/;
regExMap['en_US']['departureDate'] = /Check-out *([1-z]+ [0-9]{1,2} [A-z]+ [0-9]+) \(([0-9]{1,2}:[0-9]{2}) - ([0-9]{1,2}:[0-9]{2})\)/;
regExMap['en_US']['person'] = /Guest name +(.*) Edit guest name/;

regExMap['fr_FR'] = [];
regExMap['fr_FR']['bookingRef'] = /Numéro de réservation : ([0-9]*)\s+/;
// 1: hotel name, 2: adress, 3: city, 4:postal code, 5: country, 6: phone
regExMap['fr_FR']['hotelInformation'] = /(.+), (.+), (.+), (.+) -\s+Téléphone : (\+[0-9]*)\s+/;
regExMap['fr_FR']['hotelName'] = /L'établissement (.*) vous attend le/;
regExMap['fr_FR']['arrivalDate'] = /Arrivée  ([a-z]+ [0-9]{1,2} [a-zûé]+ [0-9]+) \(([0-9]{1,2}:[0-9]{2}) - ([0-9]{1,2}:[0-9]{2})\)/;
regExMap['fr_FR']['departureDate'] = /Départ  ([a-z]+ [0-9]{1,2} [a-zûé]+ [0-9]+) \(([0-9]{1,2}:[0-9]{2}) - ([0-9]{1,2}:[0-9]{2})\)/;
regExMap['fr_FR']['person'] = /Clients +(.*) Modifier le nom du client/;

function main(text) {
    var res = JsonLd.newObject("LodgingReservation");

    for (var locale in regExMap) {
        var bookingRef = text.match(regExMap[locale]['bookingRef']);
        // If no booking reference go to the next locale
        if (!bookingRef)
            continue;

        res.reservationNumber = bookingRef[1];

        var hotelName = text.match(regExMap[locale]['hotelName']);
        if (!hotelName)
            return null;

        res.reservationFor = JsonLd.newObject("LodgingBusiness");
        res.reservationFor.name = hotelName[1];

        var hotel = text.match(regExMap[locale]['hotelInformation']);
        if (!hotel)
            return null;

        res.reservationFor.address = JsonLd.newObject("PostalAddress");
        res.reservationFor.address.streetAddress = hotel[1];
        res.reservationFor.address.postalCode = hotel[3];
        res.reservationFor.address.addressLocality = hotel[2];
        res.reservationFor.address.addressCountry = hotel[4];
        res.reservationFor.telephone = hotel[5];

        idx = hotel.index + hotel[0].length;

        var arrivalDate = text.substr(idx).match(regExMap[locale]['arrivalDate']);
        if (!arrivalDate)
            return null;

        res.checkinTime = JsonLd.toDateTime(arrivalDate[1] + " " + arrivalDate[2], "dddd d MMMM yyyy hh:mm", locale);

        idx += arrivalDate.index + arrivalDate[0].length;

        var departureDate = text.substr(idx).match(regExMap[locale]['departureDate']);

        if (!departureDate)
            return null;
        res.checkoutTime = JsonLd.toDateTime(departureDate[1] + " " + departureDate[3], "dddd d MMMM yyyy hh:mm", locale);
        idx += departureDate.index + departureDate[0].length;


        res.underName = JsonLd.newObject("Person");
        var name = text.substr(idx).match(regExMap[locale]['person']);
        if (!name)
            return null;
        res.underName.name = name[1];


        return res;
    }
}
