/*
    SPDX-FileCopyrightText: 2025 Johannes Krattenmacher <git.noreply@krateng.ch>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

function parseConfirmation(html) {

    var res = JsonLd.newLodgingReservation();

    const content = html.root.recursiveContent;

    const formats = ["d MMM yyyy h a"];

    // Supporting English and German
    let info = content.match(/(?:Confirmation|Buchungsnummer):?\s*([0-9#]+)\n(.+)\n[\s\S]*?(.+)\n(?:Address|Adresse):\n([\s\S]*?)\n(?:Front Desk|Rezeption):\n(\d{4,25})\n(?:Email|E-Mail):\n(.*)\n(?:Dates|Daten)\n(\d{1,2} \w{3} \d{4})[\s\S]*?(\d{1,2} \w{3} \d{4})\nCheck in ‌(\d{1,2} [ap]m).* \/ Check out[\s\S]*?(\d{1,2} [ap]m)[\s\S]*?(?:Reservation|Reservierung)/)

    // sometimes includes #, sometimes not - not sure if hotel specific
    res.reservationNumber = info[1];
    res.reservationFor.name = info[2] + info[3];
    res.reservationFor.address.streetAddress = info[4];
    res.reservationFor.telephone = info[5];
    res.reservationFor.email = info[6];

    let checkin = info[7] + " " + info[9];
    let checkout = info[8] + " " + info[10];

    res.checkinTime = JsonLd.toDateTime(checkin, formats, 'en');
    res.checkoutTime = JsonLd.toDateTime(checkout, formats, 'en');

    return res;
}
