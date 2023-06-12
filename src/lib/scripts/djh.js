// SPDX-FileCopyrigthText: 2023 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: MIT

function parseConfirmation(html) {
    const res = JsonLd.newLodgingReservation();
    const elements = html.eval('/html/body/span/table/tr/td/table/tr/td/nobr');
    for (let index in elements) {
        const element = elements[index];
        if (element.content.startsWith('Telefon:')) {
            res.reservationFor.telephone = element.content.match(/^Telefon: (.*)/)[1];
        } else if (element.content.startsWith('Fax:')) {
            res.reservationFor.faxNumber = element.content.match(/^Fax: (.*)/)[1];
        } else if (element.content.startsWith('E-Mail:')) {
            res.reservationFor.email = element.content.match(/^E-Mail: (.*)/)[1];
        } else if (element.content.startsWith('Internet:')) {
            res.reservationFor.url = element.content.match(/^Internet: (.*)/)[1];
        } else if (element.content.startsWith('Datum:')) {
            res.bookingTime = JsonLd.toDateTime(element.content.match(/^Datum: (.*)/)[1], 'dd.MM.yyyy', 'de');
        } else if (index == 1) {
            res.reservationFor.address.name = element.content;
            res.name = element.content
        } else if (index == 2) {
            res.reservationFor.address.streetAddress = element.content;
        } else if (index == 3) {
            const match = element.content.match(/D-([0-9]*)(.*)/)
            res.reservationFor.address.postalCode = match[1];
            res.reservationFor.address.addressLocality = match[2];
        }
    }

    res.reservationNumber = html.rawData.match(/Ihre Buchung Nr. ([0-9]*)/)[1];

    const dates = html.eval('/html/body/span/table[@class="S_T_100"]/tr/td[@class="S_T_50"]/b');

    const time = html.rawData.match(/Anreisetag ab (.*) Uhr, am Abreisetag bis (.*) Uhr/)

    res.checkinTime = JsonLd.toDateTime(dates[0].content + ' ' + time[1], 'dd.MM.yyyy H:mm', 'de');
    res.checkoutTime = JsonLd.toDateTime(dates[1].content + ' ' + time[2], 'dd.MM.yyyy h:mm', 'de');

    res.reservationStatus = "ReservationConfirmed";
    res.lodgingUnitDescription = dates[2].content;

    res.totalPrice = dates[4].recursiveContent.match(/[,0-9]*/)[0];
    res.priceCurrency = "EUR";
    const bookedUnder = html.eval('/html/body/span/table/tr/td/table/tr/td[@style="vertical-align: top"]')[0].recursiveContent.split('\n');

    res.underName.name = bookedUnder[0].trim() + ' ' + bookedUnder[2].trim();

    res.programMembershipUsed = {
        '@type': "ProgramMembership",
        hostingOrganization: "Deutsches Jugendherbergswerk Hauptverband für Jugendwandern und Jugendherbergen e.V."
    };

    // Count reservation
    res.numAdults = 0;
    const soloTravelerCount = parseInt(html.eval('/html/body/span/table[@class="S_T_100"]/tr/td[@class="S_T_50"]/nobr')[0].content.match(/([0-9]*) Einzelreisende/))

    if (soloTravelerCount > 0) {
        res.numAdults += soloTravelerCount;
    }

    // TODO find reservation for childs

    return res;
}
