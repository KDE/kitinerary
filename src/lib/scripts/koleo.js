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

function convertTripPlanToData(data) {
    // On the ticket there is no arrival date :shrug:
    return {
        departureDate: data[0],
        departureTime: data[1],
        arrivalDate: data[0],
        arrivalTime: data[data.length - 2],
        train: data[data.length - 1 ]
    };
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
        SeatType: match[5],
        trainNumber: match[6],
        SeatSection: match[7],
        SeatNumber: match[8]
    };
}

function extractPdfTicket(content, node) {
    const reservations = [];

    const contentLines = removeEmptyElements(content.text.split('\n'));

    // Find the number of routes and data I need
    const startRegex = new RegExp(`\\bBILET\\b`, 'g');
    const validity = new RegExp(`\\bWażny od \\b`, 'g');
    const tripPlan = new RegExp(`\\bPODRÓŻ TAM\\b`, 'g');
    const priceRegex = new RegExp(`\\bCena\\b`, 'g');
    const endRegex = new RegExp(`\\bInformacja o cenie\\b`, 'g');

    const matches = [];
    let startIndex = -1;
    let validityIndex = -1
    let tripPlanIndex = -1
    let priceIndex = -1

    contentLines.forEach((line, index) => {
        if (startRegex.test(line)) {
            startIndex = index;
        } else if (startIndex !== -1 && validity.test(line)) {
            validityIndex = index;
        } else if (startIndex !== -1 && tripPlan.test(line)) {
            tripPlanIndex = index+1;
        } else if (startIndex !== -1 && priceRegex.test(line)) {
            priceIndex = index;
        } else if (startIndex !== -1 && endRegex.test(line)) {
            matches.push({ start: startIndex, validity: validityIndex, tripPlan: tripPlanIndex, price: priceIndex, end: index });
            startIndex, tripPlanIndex, priceIndex = -1;
        }
    });

    console.log(`Number of routes: ${matches.length}`);

    matches.forEach((thisTicket, index) => {
        const reservation = JsonLd.newTrainReservation();

        const UICTable = convertUICTableToData(contentLines.slice(thisTicket.start, thisTicket.end).join('\n'))
        // If the table has all the date, it won't be null, so i can just assign things... Probably its going to be PKP
        if (UICTable) {
            // There is no way to get to know for what year is this ticket for...
            reservation.reservationFor.departureTime = JsonLd.toDateTime(UICTable.departureTime, "dd.MM hh:mm", "pl");
            reservation.reservationFor.departureStation.name = UICTable.departureStation;
            reservation.reservationFor.arrivalStation.name = UICTable.arrivalStation;
            reservation.reservationFor.arrivalTime = JsonLd.toDateTime(UICTable.arrivalTime, "dd.MM hh:mm", "pl");
            reservation.reservedTicket.ticketedSeat.seatingType = UICTable.SeatType;
            reservation.reservationFor.trainNumber = UICTable.trainNumber;
            reservation.reservedTicket.ticketedSeat.seatSection = UICTable.SeatSection;
            reservation.reservedTicket.ticketedSeat.seatNumber = UICTable.SeatNumber;
        } else {

            // Year is only accessible when it says how long ticket is valid (but is not necessary travel time)
            const dateRegex = /\b\d{2}\.\d{2}\.\d{4}\b/g;
            const year = contentLines[thisTicket.validity].match(dateRegex)[0].split(".")[2];

            // if a train station has more than 1 word, it splits it to 3 lines, so i just look where is the `->` cuz it has train number
            let realTimeAndTrainNo= contentLines[thisTicket.tripPlan].split(/\s{2,}/)
            if (!realTimeAndTrainNo.includes("->")){
                realTimeAndTrainNo= contentLines[thisTicket.tripPlan+1].split(/\s{2,}/)
            }
            const routeData = convertTripPlanToData(realTimeAndTrainNo);

            reservation.reservationFor.trainNumber = routeData.train;
            reservation.reservationFor.departureTime = JsonLd.toDateTime(routeData.departureTime + routeData.departureDate + "." + year, 'hh:mmdd.MM.yyyy', "pl")
            reservation.reservationFor.arrivalTime = JsonLd.toDateTime(routeData.arrivalTime + routeData.arrivalDate + "." + year, 'hh:mmdd.MM.yyyy', "pl")

            // Takes departure and arrival station from the UIC table
            const stations = removeEmptyElements(contentLines[thisTicket.start+2].replace("->", "").split(/\s{2,}/));
            reservation.reservationFor.departureStation.name = stations[2];
            reservation.reservationFor.arrivalStation.name = stations[3];


            // Splits the line with price and the currency
            const lineWithPrice= contentLines[thisTicket.price].split(/\s{2,}/)
            const price = lineWithPrice[lineWithPrice.length-1].split(" ")

            reservation.reservedTicket.totalPrice= price[0].replace(",", ".")
            reservation.reservedTicket.priceCurrency = price[1]


        }
        // text is always last, before its ONLY codes
        const thisCode = node.childNodes[index].childNodes[0].content

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
