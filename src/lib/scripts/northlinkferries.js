/*
 * SPDX-FileCopyrightText: 2025 Jonas Junker <jonassimonjunker@proton.me>
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

function main(html) {
    // get the text from the booking confirmation e-mail
    const text = html.root.recursiveContent;
    // create new ferry booking
    let res = JsonLd.newBoatReservation();

    // ports and times
    const date_route = text.match(/\s+DATE ROUTE SHIP DEPART ARRIVE\s+(\w+) (\d{1,2} \S+ \d{4}) (\S+) to (\S+) (\S+) (\d{2}:\d{2}) (\d{2}:\d{2})/);
    const departure_terminal = ports[date_route[3]];
    const arrival_terminal = ports[date_route[4]];
    res.reservationFor.departureBoatTerminal.name = departure_terminal.name;
    res.reservationFor.departureBoatTerminal.geo.latitude = departure_terminal.coords[0];
    res.reservationFor.departureBoatTerminal.geo.longitude = departure_terminal.coords[1];
    res.reservationFor.departureBoatTerminal.address.streetAddress = departure_terminal.streetaddress;
    res.reservationFor.departureBoatTerminal.address.addressLocality = departure_terminal.city;
    res.reservationFor.departureBoatTerminal.address.postalCode = departure_terminal.postalCode;
    res.reservationFor.departureBoatTerminal.address.addressRegion = departure_terminal.region;
    res.reservationFor.departureBoatTerminal.address.addressCountry = departure_terminal.country;
    res.reservationFor.departureTime = JsonLd.toDateTime(date_route[2] + ' ' + date_route[6], 'd MMMM yyyy hh:mm' ,'en');

    res.reservationFor.arrivalBoatTerminal.name = arrival_terminal.name;
    res.reservationFor.arrivalBoatTerminal.geo.latitude = arrival_terminal.coords[0];
    res.reservationFor.arrivalBoatTerminal.geo.longitude = arrival_terminal.coords[1];
    res.reservationFor.arrivalBoatTerminal.address.streetAddress = arrival_terminal.streetaddress;
    res.reservationFor.arrivalBoatTerminal.address.addressLocality = arrival_terminal.city;
    res.reservationFor.arrivalBoatTerminal.address.postalCode = arrival_terminal.postalCode;
    res.reservationFor.arrivalBoatTerminal.address.addressRegion = arrival_terminal.region;
    res.reservationFor.arrivalBoatTerminal.address.addressCountry = arrival_terminal.country;
    res.reservationFor.arrivalTime = JsonLd.toDateTime(date_route[2] + ' ' + date_route[7], 'd MMMM yyyy hh:mm' ,'en');

    // look for cabins
    const cabins = text.match(/Cabins\s+([A-Za-z ]+)/);
    // if there is cabins booked and the departure_terminal is Stromness, the user has booked the Hamnavoe B&B 
    // https://www.northlinkferries.co.uk/booking-info/offers/hamnavoe-bed-and-breakfast/ 
    // and sleeps already aboard the ferry the night prior the sailing  
    // --> set the departureTime to the day before to 21:30 (earliest checkin time).
    if (cabins !== null && date_route[3] == "Stromness") {
        console.log("Hamnavoe B&B booked");
        res.reservationFor.departureTime.setDate(res.reservationFor.departureTime.getDate() - 1);
        res.reservationFor.departureTime.setHours(21);
        res.reservationFor.departureTime.setMinutes(30);
        console.log(res.reservationFor.departureTime)
    }

    // reserving persons
    const passengers = text.match(/([A-Za-z ]+) (\w+) (\w{4,6}) £(\d+.\d+)/g);
    console.log(passengers);

    // reservation number
    const reservationNumber = text.match(/\s+Your booking reference number is (\S+)\s+/);
    res.reservationNumber = reservationNumber[1];

    // booking time (not yet implemented in the boat schema )
    // const dateofsale = text.match(/Date of Sale:\s+([0-9.]+)/);
    //res.reservationFor.bookingTime = JsonLd.toDateTime(dateofsale[1] + "00:00", "dd.mm.yyyy hh:mm  ", "en");

    // parse passenger information
    let idx = 0;
    let reservations = [res];

    while (true) {
        const pas = text.substr(idx).match(/([A-Za-z ]+) (\w+) (\w{4,6}) £(\d+.\d+)/)
        if (!pas)
            break;
        console.log(pas[1])
        idx += pas.index + pas[0].length;
        let r = JsonLd.clone(res);
        r.underName.name = pas[1];
        reservations.push(r);
    }
    return reservations
}

var ports = {
        "Aberdeen": {name: "Aberdeen Ferry Terminal",
            streetaddress: "Jamieson’s Quay",
            city: "Aberdeen",
            region: "Aberdeenshire",
            postalCode: "AB11 5NP",
            country: "United Kingdom",
            coords: [57.1449840, -2.0914130]},
        "Kirkwall": {name: "Hatston Ferry Terminal, Kirkwall",
            streetaddress: "Hatston Quay",
            city: "Kirkwall",
            region: "Orkney",
            postalCode: "KW15 1RQ",
            country: "United Kingdom",
            coords: [58.9999330, -2.9741150]},
        "Lerwick": {name: "Holmsgarth Ferry Terminal",
            streetaddress: "Holmsgarth Road",
            city: "Lerwick",
            region: "Shetland",
            postalCode: "ZE1 0PR",
            country: "United Kingdom",
            coords: [60.1625570, -1.1590000]},
        "Scrabster": {name: "Scrabster Ferry Terminal",
            streetaddress: "Queen Elizabeth Pier",
            city: "Scrabster",
            region: "Caithness",
            postalCode: "KW14 7UW",
            country: "United Kingdom",
            coords: [58.6124540, -3.5396220]},
        "Stromness": {name: "Stromness Ferry Terminal",
            streetaddress: "Ferry Road",
            city: "Stromness",
            region: "Orkney",
            postalCode: "KW16 3BH",
            country: "United Kingdom",
            coords: [58.9640600, -3.2956710]},
};
