/*
   SPDX-FileCopyrightText: 2026 Stephan Seitz <stephan.seitz@fau.de>
   SPDX-License-Identifier: LGPL-2.0-or-later
*/

function decodeBarcode(page, barcode) {
    // top left quarter page contains price, billing info and general terms and conditions
    var billingInfo = page.textInRect(0.0, 0.0, 0.5, 0.5);
    // top right quarter page contains all travel info
    var travelInfo = page.textInRect(0.5, 0.0, 1.0, 0.5);

    var departureAddressText = page.textInRect(0.5, 0.18, 0.75, 0.25).split(",\n");
    var arrivalAddressText = page.textInRect(0.75, 0.18, 1.0, 0.25).split(",\n");

    // example Madrid->Bilbao ASP=N202608261600&Q=:3S000000**B203-1-999-32337258-1**$;203-1-999-32337258-1-1*VAC-157*3885*480200001*280790001*202608252007*202608261600 
    const code = barcode.split('*');
    if (code.length != 11 || !barcode.startsWith("ASP=")){
        console.log("Barcode didn't have the expected format:", barcode);
        return null;
    }

    // The departureCode and destinationCode in the barcode can only be decoded using a data from INE (derived from https://www.ine.es/daco/daco42/codmun/codmun08/08codmun.xls, CC-BY-4.0)
    // We decided to not embed this mapping here, see discussion in https://invent.kde.org/pim/kitinerary/-/merge_requests/236

    const departureTime = JsonLd.toDateTime(code[0].match(/ASP=N(\d{12}).*/)[1], ["yyyyMMddhhmm"], "en");
    // const empty = code[1];
    const ticketId = code[2];
    // const empty2 = code[3];
    // const ticketId = code[4];
    // const concessionCode = code[5];
    const lineNumber = code[6];
    const departureCode = code[7];
    const destinationCode = code[8];
    //const bookingTime = code[9];
    //const departureTimeAgain = code[10];

    // Arrival time only in text
    const dates = travelInfo.match(/(\d+\s\w+\s\d{4})\s+(\d+\s\w+\s\d{4})/);
    const times = travelInfo.match(/(\d+:\d+)\s+(\d+:\d+)/);

    var arrivalTime = departureTime;
    if (dates && times) {
        arrivalTime = JsonLd.toDateTime(dates[2] + " " + times[2], ["dd MMMM yyyy hh:mm"], ["es", "en"]);
    }

    var res = JsonLd.newBusReservation();
    res.reservationFor.departureTime = departureTime;
    res.reservationFor.arrivalTime = arrivalTime;
    res.reservationFor.provider.name = "ALSA";

    res.reservationId = ticketId;
    res.reservationFor.busNumber = lineNumber;

    res.reservationFor.departureBusStop.identifier = "alsa:" + departureCode;
    if (departureAddressText.length >= 3) {
        const busStopName = departureAddressText[0].replace(/,$/, "").trim();
        const streetAddress = departureAddressText[1].replace(/,$/, "").trim();
        const city = departureAddressText[2].split("\n\n")[0].replace(/,$/, "").trim();

        // e.g. Madrid (INTERCAMBIADOR DE AUTOBUSES)
        res.reservationFor.departureBusStop.name = city + " (" + busStopName + ")";
        res.reservationFor.departureBusStop.address.streetAddress = streetAddress;
        res.reservationFor.departureBusStop.address.addressLocality = city;
    }
    
    res.reservationFor.arrivalBusStop.identifier = "alsa:" + destinationCode;
    if (arrivalAddressText.length >= 3) {
        const busStopName = arrivalAddressText[0].replace(/,$/, "").trim();
        const streetAddress = arrivalAddressText[1].replace(/,$/, "").trim();
        const city = arrivalAddressText[2].split("\n\n")[0].replace(/,$/, "").trim();

        res.reservationFor.arrivalBusStop.name = city + " (" + busStopName + ")";
        res.reservationFor.arrivalBusStop.address.streetAddress = streetAddress;
        res.reservationFor.arrivalBusStop.address.addressLocality = city;
    }

    const totalPrice = billingInfo.match(/Importe total: (\d+,\d+€)/);
    if (totalPrice) {
        res.totalPrice = totalPrice[1];
    }

    const name = travelInfo.match(/^(.*)\s,\s(.*)$/m);
    if (name) {
        res.underName.familyName = name[1].trim();
        res.underName.givenName = name[2].trim();
    }

    const seatInfo = travelInfo.match(/Autobús\s+Asiento\s+(\d+)\s+(\d+)/);
    if (seatInfo) {
        res.reservedTicket.ticketedSeat.seatSection = seatInfo[1]; // coach number
        res.reservedTicket.ticketedSeat.seatNumber = seatInfo[2];
    }

    res.reservedTicket.ticketToken = "qrCode:" + barcode;

    return res;
}

function parsePdfTicket(pdf, node, triggerNode)
{
    let reservations = [];
    for (var i = 0; i < pdf.pageCount; ++i) {
        var page = pdf.pages[i];
        var nextBarcode = null;
        var images = page.images;
        for (var j = 0; j < images.length && !nextBarcode; ++j) {
            nextBarcode = Barcode.decodeQR(images[j]);
            if (nextBarcode)
                reservations.push(decodeBarcode(page, nextBarcode));
        }
    }
    return reservations
}
