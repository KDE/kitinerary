/*
   SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
   SPDX-FileCopyrightText: 2025 Carl Schwan <carl@carlschwan.eu>
   SPDX-License-Identifier: LGPL-2.0-or-later
*/

function parseMembershipCard(code) {
    console.log(code);
    const c = code.split(/;/);
    let card = {
        '@type': 'ProgramMembership',
        programName: 'DJH Mitgliedskarte (' + c[10] + ')',
        membershipNumber: c[0].substr(1),
        member: {
            '@type': 'Person',
            givenName: c[2],
            familyName: c[3],
        },
        token: 'dataMatrix:' + code,
        validFrom: JsonLd.toDateTime(c[12], 'dd.MM.yyyy?', 'de'),
        validUntil: JsonLd.toDateTime(c[11], 'MM/yyyy', 'de'),
    };
    return card;
}

function parseReservation(pdf) {
    const pdfText = pdf.text;
    const res = JsonLd.newLodgingReservation();

    var num = pdfText.match(/Reservierungs-Nr. (.+)/)[1];
    res.reservationNumber = num.split(".").join("");

    res.reservationFor.address.addressCountry = "Germany";
    let nextIsName = false;
    let i = 0;
    for (let line of pdf.text.split('\n')) {
        console.log(i, line)
        if (i === 0) {
            res.reservationFor.name = line.trim();
        }
        if (i === 2) {
            res.reservationFor.address.streetAddress = line.trim();
        }
        if (i === 3) {
            res.reservationFor.address.addressLocality = line.trim();
        }
        if (nextIsName)  {
            res.underName.name = line
            break;
        }
        if (line.includes("Telefon: ")) {
            nextIsName = true;
        }
        i++;
    }

    const info = pdfText.match(/Erste Mahlzeit\n([^ ]+)\s+([^ ]+)\s+/);
    const checkinTime = pdfText.match(/Check-in: ([^ ]+)\s+/);
    const checkoutTime = pdfText.match(/Check-out: [^ ]+ - ([^ ]+)/);

    res.checkinTime = JsonLd.toDateTime(info[1] + ' ' + checkinTime[1], "dd.MM.yy hh:mm", 'de');
    res.checkoutTime = JsonLd.toDateTime(info[2] + ' ' + checkoutTime[1], "dd.MM.yy hh:mm", 'de');

    const price = pdfText.match(/Gesamtpreis für den oben genannten Buchungszeitraum:\s+ (.+) EURO/);

    res.totalPrice = price[1].replace(",", ".");

    return res;
}
