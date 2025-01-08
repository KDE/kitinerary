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

function isValidJSON(str) {
    try {
        JSON.parse(str);
        return true;
    } catch (e) {
        return false;
    }
}

// This is pure guess, will it work? i have no idea X'D
function validateStartHex(string, hexValues) {
    if (string.length < hexValues.length) {
        return false;
    }

    for (let i = 0; i < hexValues.length; i++) {
        if (string.charCodeAt(i) !== hexValues[i]) {
            return false;
        }
    }

    return true;
}

function removeEmptyElements(arr) {
    return arr.filter(element => element !== undefined && element !== null && element !== '').filter(Boolean);
}

function convertLineToData(data) {
    return {
        id: data[0],
        departureDate: data[1],
        departureTime: data[2],
        departureStation: data[3],
        arrivalStation: data[4],
        arrivalDate: data[5],
        arrivalTime: data[6],
        train: data[7],
        class: data[8],
        coach: data[9],
        seat: data[10]
    };
}


function main(content, node) {
    const reservations = [];

    // Just so its easier to parse, without too many spaces
    const contentLines = content.text.split('\n')

    // Determine the amount routes mentioned in the ticket
    let numberOfRoutes = 0;
    while (numberOfRoutes < contentLines.length) { // Probably we will never go out of bound but :shrug:
        let currentTextLine = removeEmptyElements(contentLines[2 + numberOfRoutes].split(/\s{2,}/))[0]
        if (currentTextLine == numberOfRoutes + 1) {
            numberOfRoutes++;
        } else {
            break;
        }
    }

    console.log(`Number of routes: ${numberOfRoutes}`);

    for (let i = 1; i <= numberOfRoutes; i++) {

        const reservation = JsonLd.newTrainReservation();
        const baseIndex = 1 + i;

        // Split where the discriminator is "2 or more spaces"
        const thisRoute = removeEmptyElements(contentLines[baseIndex].split(/\s{2,}/))
        const routeData = convertLineToData(thisRoute);


        reservation.reservationFor.departureStation.name = routeData.departureStation;
        reservation.reservationFor.arrivalStation.name = routeData.arrivalStation;
        reservation.reservationFor.trainNumber = routeData.train;
        reservation.reservedTicket.ticketedSeat.seatingType = routeData.class;
        reservation.reservedTicket.ticketedSeat.seatSection = routeData.coach;
        reservation.reservedTicket.ticketedSeat.seatNumber = routeData.seat;


        reservation.reservationFor.departureTime = JsonLd.toDateTime(routeData.departureTime + routeData.departureDate, 'hh:mmdd.MM.yyyy', "pl")
        reservation.reservationFor.arrivalTime = JsonLd.toDateTime(routeData.arrivalTime + routeData.arrivalDate, 'hh:mmdd.MM.yyyy', "pl")

        // im blindly guessing that it is 1 line under all routes.
        const ticketPrice = contentLines[1+numberOfRoutes+1].split(/\s{2,}/)[1].split(' ');
        reservation.reservedTicket.priceCurrency = ticketPrice[2];
        reservation.reservedTicket.totalPrice = ticketPrice[1].replace(",", ".");

        // Im guessing its "i"th child node
        const thisCode = node.childNodes[i].childNodes[0].content

        // PKP IC and Koleje śląskie are tested, rest no :c
        if ( hexToText(thisCode).substring(0, 2)=="32"){
            reservation.reservedTicket.ticketToken = 'aztecBin:' +  ByteArray.toBase64(thisCode);
            console.log("Probably PKP IC")
        } else if ( thisCode.substring(0, 2) == "ED" || thisCode.substring(0, 2) == "EH"  ) {
            // ED and EH is taken from 2 tickets I have, maybe its not good way to find it?
            reservation.reservedTicket.ticketToken = 'qrcode:' +  thisCode;
            console.log("Probably Koleje Śląske") // KŚ QR has error correction 30% while Itinerery makes 25%
        } else if ( isValidJSON(thisCode) ){
            reservation.reservedTicket.ticketToken = 'qrcode:' +  thisCode;
            console.log("Probably Koleje Małopolskie")
        } else if ( validateStartHex(thisCode, [0xA9, 0xD2]) ){
            // Based on one example, it could be completely wrong
            reservation.reservedTicket.ticketToken = 'datamatrix:' +  thisCode;
            console.log("Probably Polregio")
        } else if ( validateStartHex(thisCode, [0x03, 0x01, 0x20]) ){
            // https://community.kde.org/KDE_PIM/KItinerary/Koleje_Mazowiekie
            reservation.reservedTicket.ticketToken = 'qrcode:' +  thisCode;
            console.log("Probably Koleje Mazowieckie")
        } else if ( node.childNodes[i].childNodes[0]?.childNodes[0]?.mimeType == "internal/uic9183" ) { // idk how bad are the "?" but... maybe fixme?
            reservation.reservedTicket.ticketToken = 'atzecbin:' + ByteArray.toBase64(node.childNodes[i].childNodes[0].childNodes[0].content);
            console.log("Probably Koleje Dolnośląskie")
        } else {
            console.log("I got no idea who make this ticket... try reporting issue :) ")
        }


        reservations.push(reservation);
    }

    return reservations;
}
