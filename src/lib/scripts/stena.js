/*
    SPDX-FileCopyrightText: 2025 Johannes Krattenmacher <git.noreply@krateng.ch>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

function parseConfirmation(html) {
    const content = html.root.recursiveContent;
    let res = JsonLd.newBoatReservation();
    res.reservationNumber = content.match(/BOOKING REFERENCE:.*\n.*?(\d+)/)[1];

    var timeinfo = content.match(/Departs\n.*(\d\d\/\d\d\/\d\d\d\d).*\n.*(\d\d:\d\d)\n.*Arrives\n.*(\d\d\/\d\d\/\d\d\d\d).*\n.*(\d\d:\d\d)/);

    res.reservationFor.departureTime = JsonLd.toDateTime(timeinfo[1] + " " + timeinfo[2], "dd/MM/yyyy hh:mm", "en");
    res.reservationFor.arrivalTime = JsonLd.toDateTime(timeinfo[3] + " " + timeinfo[4], "dd/MM/yyyy hh:mm", "en");

    var routeinfo = content.match(/Route Information\n(.+)-(.+)/);
    var departPort = routeinfo[1];
    var arrivePort = routeinfo[2];

    // hardcode ports that Stena Line uses?
    // all but liepaja and travemunde are NOT confirmed to use this exact string on the ticket
    var ports = {
        'Liepaja': [56.529230, 20.998445, "14A Brīvostas Street", 3405, "Liepāja", "Stena Line Liepāja Ferry Terminal"],
        'Travemünde': [53.940075, 10.852651, "Zum Hafenplatz 1", 23570, "Lübeck-Travemünde", "Skandinavienkai"],
        'Frederikshavn': [57.43459, 10.54365, "Færgehavnsvej 10", 9900, "Frederikshavn", "Stena Line Denmark A/S"],
        'Gothenburg': [57.701195, 11.947163, "Emigrantvägen 20", 41327, "Gothenburg", "Denmark Terminal"],
        'Hook of Holland': [51.974122, 4.128671, "Stationsweg 10", 3151, "Hook of Holland", "Stena Line BV"],
        'Harwich': [51.946494, 1.252731, "Parkeston Quay", "CO12 4SR", "Harwich", "Harwich International Port"],
        'Ventspils': [57.398297, 21.569751, "Dārzu iela 6", 3601, "Ventspils", "Pramju Terminalis"],
        'Nynäshamn': [58.93915, 17.97527, "Norvikvägen 26", 14945, "Nynäshamn", "Norviks färjeterminal"],
        'Rostock': [54.141775, 12.104687, "Zum Fährterminal 1", 18147, "Rostock", "Fährterminal 1"],
        'Trelleborg': [55.373026, 13.154049, "Norra Nyhamnsgatan 1A", 23161, "Trelleborg", "Norra Nyhamnsgatan 1A"],
        'Gdynia': [54.533212, 18.544250, "ul. Polska 4", 81339, "Gdynia", "Gdynia Port"],
        'Karlskrona': [56.1646, 15.6308, "Verkövägen 101", 37165, "Lyckeby, Karlskrona", "Stena Line Scandinavia AB"],
        // more
    }

    if (departPort in ports) {
        res.reservationFor.departureBoatTerminal.geo.latitude = ports[departPort][0];
        res.reservationFor.departureBoatTerminal.geo.longitude = ports[departPort][1];
        res.reservationFor.departureBoatTerminal.address.streetAddress = ports[departPort][2];
        res.reservationFor.departureBoatTerminal.address.postalCode = ports[departPort][3];
        res.reservationFor.departureBoatTerminal.address.addressLocality = ports[departPort][4];
        res.reservationFor.departureBoatTerminal.name = ports[departPort][5];
    }
    else {
        res.reservationFor.departureBoatTerminal.name = departPort;
        res.reservationFor.departureBoatTerminal.address.addressLocality = departPort;
    }

    if (arrivePort in ports) {
        res.reservationFor.arrivalBoatTerminal.geo.latitude = ports[arrivePort][0];
        res.reservationFor.arrivalBoatTerminal.geo.longitude = ports[arrivePort][1];
        res.reservationFor.arrivalBoatTerminal.address.streetAddress = ports[arrivePort][2];
        res.reservationFor.arrivalBoatTerminal.address.postalCode = ports[arrivePort][3];
        res.reservationFor.arrivalBoatTerminal.address.addressLocality = ports[arrivePort][4];
        res.reservationFor.arrivalBoatTerminal.name = ports[arrivePort][5];
    }
    else {
        res.reservationFor.arrivalBoatTerminal.name = arrivePort;
        res.reservationFor.arrivalBoatTerminal.address.addressLocality = arrivePort;
    }

    return res;
}
