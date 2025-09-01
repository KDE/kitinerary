/*
   SPDX-FileCopyrightText: 2017 Volker Krause <vkrause@kde.org>
   SPDX-FileCopyrightText: 2018 Daniel Vrátil <dvratil@kde.org>
   SPDX-FileCopyrightText: 2025 David Pilarčík <meow@charliecat.space>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

const regExMap = new Array();
regExMap["cs_CZ"] = new Array();
regExMap["cs_CZ"]["ticketId"] = /Elektronická jízdenka č\.\s+([0-9]+)/;
regExMap["cs_CZ"]["singleTripHeader"] = /Cesta/;
regExMap["cs_CZ"]["thereTripHeader"] = /Cesta tam/;
regExMap["cs_CZ"]["returnTripHeader"] = /Cesta zpět/;
regExMap["cs_CZ"]["columns"] = [ /Datum/, /Zastávka\/Přestup/, /Příjezd/, /Odjezd/, /Nást\./, /Spoj/, /Vůz\/sedadla/ ];
regExMap["cs_CZ"]["date"] = /([0-9]{1,2})\.([0-9]{1,2})\.([0-9]{2,4}$)/;
regExMap["en_US"] = new Array();
regExMap["en_US"]["ticketId"] = /Electronic ticket\s+([0-9]+)/;
regExMap["en_US"]["singleTripHeader"] = /Route/;
regExMap["en_US"]["thereTripHeader"] = /Route there/;
regExMap["en_US"]["returnTripHeader"] = /Route back/;
regExMap["en_US"]["columns"] = [ /Date/, /Station\/Transfer/, /Arrival/, /Departure/, /Platf\./, /Connection/, /Coach\/Seats/ ];
regExMap["en_US"]["date"] = /([0-9]{2})\/([0-9]{2})\/([0-9]{2,4}$)/;

function padDigit(s) {
    while (s.length < 2) {
        s = '0' + s;
    }
    return s;
}

function parseDate(date, time, locale) {
    var d = date.match(regExMap[locale]["date"]);
    var t = time.match(/([0-9]{1,2}):([0-9]{1,2})/);
    if (!d || !t) {
        return null;
    }
    return JsonLd.toDateTime(padDigit(d[1]) + "." + padDigit(d[2]) + ".20" + d[3] + " "
                                + padDigit(t[1]) + ":" + padDigit(t[2]),
                             "dd.MM.yyyy HH:mm", locale);
}

var Columns = Object.freeze({
    Date: 0,
    Station: 1,
    ArrivalTime: 2,
    DepartureTime: 3,
    Platform: 4,
    Connection: 5,
    Seat: 6,

    ColumnCount: 7
});

function columnValue(line, columns, column, locale) {
    var start = columns.match(regExMap[locale]["columns"][column]);
    if (!start) {
        return "";
    }
    if (column < Columns.ColumnCount - 1) {
        var end = columns.match(regExMap[locale]["columns"][column + 1]);
        if (!end) {
            return "";
        }
        return line.substr(start.index, end.index - start.index).trim();
    } else {
        return line.substr(start.index).trim();
    }
}

function parseTrip(trip, locale) {
    var text = trip.split("\n")
    var columns = text[0];
    var reservations = new Array();
    var transportType = "Bus";
    for (var i = 1; i < text.length; i++) {
        // Skip the destination arrival part, we already populated it as part
        // of completing the previous departure line
        if (i < text.length - 1 && !text[i + 1]) {
            break;
        }

        var connection = columnValue(text[i], columns, Columns.Connection, locale);
        var number = null;
        var name = null;
        if (connection) {
            var split = connection.lastIndexOf("(")
            name = connection.substr(0, split - 1);
            number = connection.substr(split + 1, connection.length - split - 2);
            transportType = number.match(/RJ [0-9]+/) ? "Train" : "Bus";
        }

        var res = JsonLd.newObject(transportType + "Reservation");
        res.reservationFor = JsonLd.newObject(transportType + "Trip");
        if (transportType == "Bus") {
            if (number) {
                res.reservationFor.busNumber = number;
            }
            if (name) {
                res.reservationFor.busName = name;
            }
        } else if (transportType == "Train") {
            if (number) {
                res.reservationFor.trainNumber = number;
            }
            if (name) {
                res.reservationFor.trainName = name;
            }
        }

        var arrivalTime = columnValue(text[i + 1], columns, Columns.ArrivalTime, locale);
        var arrivalDate = columnValue(text[i + 1], columns, Columns.Date, locale);
        if (!arrivalTime) {
            arrivalTime = columnValue(text[i], columns, Columns.ArrivalTime, locale);
        }
        if (!arrivalDate) {
            arrivalDate = columnValue(text[i], columns, Columns.Date, locale);
        }
        if (arrivalDate && arrivalTime) {
            res.reservationFor.arrivalStation = JsonLd.newObject(transportType + "Station");
            res.reservationFor.arrivalStation.name = columnValue(text[i+1], columns, Columns.Station, locale);
            res.reservationFor.arrivalTime = parseDate(arrivalDate, arrivalTime, locale);
        }

        var departureTime = columnValue(text[i], columns, Columns.DepartureTime, locale);
        var departure = "";
        if (departureTime !== "") {
            departure = text[i];
        } else if (i > 0) {
            departure = text[i - 1];
            departureTime = columnValue(departure, columns, Columns.DepartureTime, locale);
        }
        if (departure) {
            res.reservationFor.departureStation = JsonLd.newObject(transportType + "Station");
            res.reservationFor.departureStation.name = columnValue(departure, columns, Columns.Station, locale);
            res.reservationFor.departureTime = parseDate(columnValue(departure, columns, Columns.Date, locale),
                                                         departureTime, locale);

            var platform = columnValue(departure, columns, Columns.Platform, locale);
            if (platform) {
                res.reservationFor.departurePlatform = platform;
            }

            // seats are always bound to departur
            var seat = columnValue(departure, columns, Columns.Seat, locale);
            if (seat) {
                var r = seat.match(/([0-9]+)\/([0-9]+)/);
                res.reservedTicket = JsonLd.newObject("Ticket");
                res.reservedTicket.ticketedSeat = JsonLd.newObject("Seat");
                if (r) {
                    res.reservedTicket.ticketedSeat.seatSection = r[1];
                    res.reservedTicket.ticketedSeat.seatNumber = r[2];
                } else {
                    res.reservedTicket.ticketedSeat.seatNumber = seat;
                }
            }
        }
        reservations.push(res);
    }
    return reservations;
}

function main(text) {
    var reservations = new Array();

    for (var locale in regExMap) {
        var ticketId = text.match(regExMap[locale]["ticketId"]);
        if (!ticketId) {
            continue;
        }

        var resUrl = text.match(/http(s)?:\/\/jizdenky\.(regiojet|studentagency)\.cz\/OnlineTicket\?pam1=[0-9]+\&pam2=[0-9]+/)

        var returnHeader = text.match(regExMap[locale]["returnTripHeader"]);
        var isReturn = (returnHeader !== null);
        var routeHeader = text.match(regExMap[locale][isReturn ? "thereTripHeader" : "singleTripHeader"]);
        if (!routeHeader) {
            break;
        }

        var trip = text.substr(routeHeader.index + routeHeader[0].length + 1);
        var newRes = parseTrip(trip, locale);
        if (newRes.length === 0) {
            break;
        }
        reservations = reservations.concat(newRes);

        if (isReturn) {
            trip = text.substr(returnHeader.index + returnHeader[0].length + 1);
            reservations = reservations.concat(parseTrip(trip, locale));
        }

        for (var i = 0; i < reservations.length; ++i) {
            reservations[i].reservationNumber = ticketId[1];
            if (resUrl) {
                reservations[i].modifyReservationUrl = resUrl[0];
            }
        }

        // No need to scan any further locales
        break;
    }

    return reservations;
}

function parseEvent(event)
{
    var res = JsonLd.newTrainReservation();
    res.reservationFor.departureTime = JsonLd.readQDateTime(event, "dtStart");
    res.reservationFor.arrivalTime = JsonLd.readQDateTime(event, "dtEnd");
    res.underName.name = event.attendees[0].name;

    var summary = event.summary.match(/#(.+?): From (.+?), to (.+?), seat: (.+)/);
    res.reservationNumber = summary[1];
    res.reservationFor.departureStation.name = summary[2];
    res.reservationFor.arrivalStation.name = summary[3];
    res.reservedTicket.ticketedSeat.seatNumber =summary[4];

    var desc = event.description.match(/\((.+?)\)/);
    res.reservationFor.trainNumber = desc[1];

    var loc = event.location.match(/(.+?), (.+)/);
    res.reservationFor.departureStation.geo.latitude = 1.0 * loc[1];
    res.reservationFor.departureStation.geo.longitude = 1.0 * loc[2];
    return res;
}

function parseDistribusionPkPass(pass, node) {
    let res = JsonLd.newBusReservation();
    res.reservedTicket = node.result[0].reservedTicket;
    res.reservationFor.departureBusStop.name = pass.field["from"].label;
    res.reservationFor.departureTime = JsonLd.toDateTime(pass.field["departure_date"].value + pass.field["boarding_time"].value, "MMMddhh:mm", "en");
    res.reservationFor.departurePlatform = pass.field["platform"].value;
    res.reservationFor.arrivalBusStop.name = pass.field["to"].label;
    res.reservationNumber = pass.field["booking_number"].value;
    res.underName.name = pass.field["passenger"].value;
    res.reservedTicket.ticketedSeat = {
        "@type": "Seat",
        seatNumber: pass.field["seat_number"].value
    };
    res.reservationFor.busName = pass.field["route"].value;
    res.reservationFor.provider.name = pass.field["operated_by"].value;
    return res;
}

function parsePkPass(pass, node) {
	let res = node.result[0]

	res.reservationNumber = pass.field["ticket-number"].value;
	res.reservedTicket.name = pass.field["tariff"].value;
	res.totalPrice = pass.field["price"].value;
	res.priceCurrency = pass.field["price"].currencyCode;

	if (pass.field["seat"].label == "Car/Seat") {
		let [car, seat] = pass.field["seat"].value.split("/");
		res.reservedTicket.ticketedSeat = {
			"@type": "Seat",
			seatNumber: seat,
			seatSection: car,
			seatingType: pass.field["tariff"].value,
		}
	} else {
		res.reservedTicket.ticketedSeat = {
			"@type": "Seat",
			seatNumber: pass.field["seat"].value,
			seatingType: pass.field["tariff"].value,
		}
	}

	if (res["@type"] == "TrainReservation") {
		res.reservationFor.departureStation = JsonLd.newObject("TrainStation");
		res.reservationFor.departureStation.name = `${pass.field["departure-destination"].value}, ${pass.field["departure-destination"].label}`;
		res.reservationFor.departureTime = pass.field["departure-time"].value
		res.reservationFor.departurePlatform = pass.field["platform"].value;

		res.reservationFor.arrivalStation = JsonLd.newObject("TrainStation");
		res.reservationFor.arrivalStation.name = `${pass.field["arrival-destination"].value}, ${pass.field["arrival-destination"].label}`;
		res.reservationFor.arrivalTime = pass.field["arrival-time"].value
	} else if (res["@type"] == "BusReservation") {
		res.reservationFor.departureBusStop = JsonLd.newObject("BusStation");
		res.reservationFor.departureBusStop.name = `${pass.field["departure-destination"].value}, ${pass.field["departure-destination"].label}`;
		res.reservationFor.departureTime = pass.field["departure-time"].value
		res.reservationFor.departurePlatform = pass.field["platform"].value;

		res.reservationFor.arrivalBusStop = JsonLd.newObject("BusStation");
		res.reservationFor.arrivalBusStop.name = `${pass.field["arrival-destination"].value}, ${pass.field["arrival-destination"].label}`;
		res.reservationFor.arrivalTime = pass.field["arrival-time"].value
	}

	return res;
}

function parseSeatReservationPdf(pdf, node, barcode) {
    const text = pdf.pages[barcode.location].text;
    let res = JsonLd.newTrainReservation();
    res.reservedTicket.ticketToken = 'qrcode:' + barcode.content;
    res.reservationNumber = text.match(/reservation nr. (\d+)\n/)[1];
    const trip = text.match(/(\d{1,2}\/\d{1,2}\/\d{2})\n(\d{1,2}:\d{2}[AP]M)\s+(\S.*)\n(\d{1,2}:\d{2}[AP]M)\s+(\S.*)\nCoach\/Seats:\s+(\S+)\/(\S+)\n.*\nClass:\s+(\S.*)\nConnection:\s+.*\(.*,(.*)\)\n/);
    res.reservationFor.departureTime = JsonLd.toDateTime(trip[1] + ' ' + trip[2], ["M/d/yy h:mmAP"], ["en"]);
    res.reservationFor.departureStation.name = trip[3];
    res.reservationFor.arrivalTime = JsonLd.toDateTime(trip[1] + ' ' + trip[4], ["M/d/yy h:mmAP"], ["en"]);
    res.reservationFor.arrivalStation.name = trip[5];
    res.reservedTicket.ticketedSeat.seatSection = trip[6];
    res.reservedTicket.ticketedSeat.seatNumber = trip[7];
    res.reservedTicket.ticketedSeat.seatingType = trip[8];
    res.reservationFor.trainNumber = trip[9];
    return res;
}
