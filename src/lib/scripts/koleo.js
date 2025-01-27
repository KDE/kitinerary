/*
 *   SPDX-FileCopyrightText: 2025 Grzegorz M
 *   SPDX-License-Identifier: LGPL-2.0-or-later
 */

function hexToText(hex) {
    let str = '';
    for (let i = 0; i < hex.length; i += 2) {
        str += String.fromCharCode(parseInt(hex.substr(i, 2), 16));
    }
    return str;
}

function removeEmptyElements(arr) {
    return arr.filter(element => element !== undefined && element !== null && element !== '').filter(Boolean);
}

function convertUICTableToData(data) {
    // Regex from previous version (C) 2024 Volker Krause <vkrause@kde.org> LGPL-2.0-or-later
    const regex = /(\d\d\.\d\d \d\d:\d\d) +(\S.*\S) +-> +(\S.*\S) +(\d\d\.\d\d \d\d:\d\d) +(\d)\n.*\n *(\S.*?\S)  +(\d+)  +(\s.*)\n/;
    const match = data.match(regex);

    if (!match) {
        return null;
    }

    return {
        departureTime: match[1],
        departureStation: match[2],
        arrivalStation: match[3],
        arrivalTime: match[4],
        seatType: match[5],
        trainNumber: match[6],
        seatSection: match[7],
        seatNumber: match[8]
    };
}

function extractPdfTicket(pdf, node) {
    const partialUICTableRegex = /\s*\*\s*\*\s* +(\S.*\S) +-> +(.*?)\s{2,}\*\s*\*\s+(\d)\n/;
    const tripPlanRegex = /(\d\d\.\d\d) \s* (\d\d:\d\d) .* (\d\d:\d\d) \s* (\S* \d*)\n/;

    let reservations = [];

    pdf.pages.forEach((thisPage, index) => {
        const reservation = JsonLd.newTrainReservation();
        const thisTicket = thisPage.text;

        // Attempt to parse using the UIC table format
        const UICData = convertUICTableToData(thisTicket);
        if (UICData) { // If UIC table is fully populated - use it
            // There is no way to get to know for what year is this ticket for...
            reservation.reservationFor.departureTime = JsonLd.toDateTime(UICData.departureTime, "dd.MM hh:mm", "pl");
            reservation.reservationFor.departureStation.name = UICData.departureStation;
            reservation.reservationFor.arrivalStation.name = UICData.arrivalStation;
            reservation.reservationFor.arrivalTime = JsonLd.toDateTime(UICData.arrivalTime, "dd.MM hh:mm", "pl");
            reservation.reservedTicket.ticketedSeat.seatingType = UICData.seatType;
            reservation.reservationFor.trainNumber = UICData.trainNumber;
            reservation.reservedTicket.ticketedSeat.seatSection = UICData.seatSection;
            reservation.reservedTicket.ticketedSeat.seatNumber = UICData.seatNumber;
        } else {
            // if UIC table is not fully populated we need to use Trip Plan
            const partialUICMatch = thisTicket.match(partialUICTableRegex);
            const tripPlanMatch = thisTicket.match(tripPlanRegex);

            reservation.reservationFor.departureStation.name = partialUICMatch[1];
            reservation.reservationFor.arrivalStation.name = partialUICMatch[2];
            reservation.reservedTicket.ticketedSeat.seatingType = partialUICMatch[3];

            reservation.reservationFor.trainNumber = tripPlanMatch[4];
            reservation.reservationFor.departureTime = JsonLd.toDateTime(tripPlanMatch[1] + tripPlanMatch[2],"dd.MMhh:mm","pl");
            reservation.reservationFor.arrivalTime = JsonLd.toDateTime(tripPlanMatch[1] + tripPlanMatch[3],"dd.MMhh:mm","pl");

            reservation.reservedTicket.ticketedSeat.seatSection = "";
            reservation.reservedTicket.ticketedSeat.seatNumber = "";
        }

        reservation.underName.name = thisPage.textInRect(0.7, 0, 1, 0.05);
        const thisCode = node.childNodes[index].childNodes[0]?.content;

        // PKP IC and Koleje śląskie are tested, rest no :c
        if ( hexToText(thisCode).substring(0, 2)=="32"){ // PKP has IC and EIC and TLK and some more...
            reservation.reservedTicket.ticketToken = 'azteccode:' +  thisCode;
        } else if ( reservation.reservationFor.trainNumber.includes("KS") || reservation.reservationFor.trainNumber.includes("KŚ") ) { // Koleje Śląskie
            reservation.reservedTicket.ticketToken = 'qrcode:' +  thisCode; // KŚ QR has 30% error correction, while Itinerery makes 25%
        } else if ( reservation.reservationFor.trainNumber.includes("KML") || reservation.reservationFor.trainNumber.includes("KMŁ") ){ // Koleje Małopolskie
            reservation.reservedTicket.ticketToken = 'qrcode:' +  thisCode;
        } else if ( reservation.reservationFor.trainNumber.includes("PR") ){ // Polregio
            reservation.reservedTicket.ticketToken = 'datamatrix:' +  thisCode;
        } else if ( reservation.reservationFor.trainNumber.includes("KL") ){ // Koleje Mazowieckie
            return; // they have their own extractor
        } else if ( node.childNodes[index].childNodes[0]?.childNodes[0]?.mimeType == "internal/uic9183" ) { // "something else"
            return; // it can be extracted by UIC extractor
        } else {
            console.log("I got no idea who make this ticket... try reporting issue :)")
            console.log("Randomly assigning QR")
            reservation.reservedTicket.ticketToken = 'qrcode:' +  thisCode
        }

        reservations.push(reservation);
    });

    return reservations;
}
