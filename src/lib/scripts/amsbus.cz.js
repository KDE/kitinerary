// SPDX-FileCopyrightText: 2024-2025 David Pilarčík <meow@charliecat.space>
// SPDX-License-Identifier: LGPL-2.0-or-later

function amsEshopCode(content, node) {
    const res = JsonLd.newBusReservation();

    const headerRegex = /seat\n([a-z A-Z 0-9 ,\s.]+)\s+([0-9:]+) ([0-9.]+)\s+([0-9-]+)\s+([0-9a-z*]+)\s+([0-9\/]+)\n(.*?)\s+([0-9:]+) ([0-9.]+)\s+(.*)/gm
    const codeRegex = /code ([A-Z0-9]{6})/gm

    let [_h, departureStation, departureTime, departureDate, service, departurePlatform, seat, arrivalStation, arrivalTime, arrivalDate, provider] = headerRegex.exec(content.text)
    let [_c, ticketCode] = codeRegex.exec(content.text)

    res.reservationNumber = ticketCode
    res.reservationFor.busNumber = service

    res.reservationFor.departureBusStop.name = departureStation
    res.reservationFor.departurePlatform = (departurePlatform !== "***") ? departurePlatform : undefined
    res.reservationFor.departureTime = JsonLd.toDateTime(departureTime + ' ' + departureDate, 'hh:mm dd.M.yyyy', 'cz');

    res.reservationFor.arrivalBusStop.name = arrivalStation
    res.reservationFor.arrivalTime = JsonLd.toDateTime(arrivalTime + ' ' + arrivalDate, 'hh:mm dd.M.yyyy', 'cz');

    res.reservedTicket.ticketedSeat.seatNumber = seat
    // res.reservedTicket.ticketToken = ticketCode

	ExtractorEngine.extractPrice(content.text, res);

    return res
}

function idosTicket(content, node, barcode) {
    const res = JsonLd.newBusReservation();

    const headerRegex = /seat\n([a-z A-Z 0-9 ,\s.]+) (.*?)\s+([0-9]{2}:[0-9]{2} \d+.\d+.\d+)\s+(.*?)\n\s+(.*?)\n([a-z A-Z 0-9 ,\s.]+) \s+([0-9]{2}:[0-9]{2} \d+.\d+.\d+)\s+(.*?)\n/
    const fareRegex = /fare type\s+.*\n(.*?)\s(\d+,[\d \-]+ .*)\s+.*?\ntřída\/class\n(.*?)\s+kód e-jízdenky: (.*?)\n/
    const idosCodeRegex = /kód IDOS.cz: (.*?)\n/

    let [_h, departureStation, departurePlatform, departureTime, seat, service, arrivalStation, arrivalTime, provider] = headerRegex.exec(content.text)
    let [_f, type, price, classType, ticketReservaitonCode] = fareRegex.exec(content.text)

    res.reservationNumber = ticketReservaitonCode
    res.reservationFor.busNumber = service

    res.reservationFor.departureBusStop.name = departureStation
    res.reservationFor.departurePlatform = (departurePlatform !== "***") ? departurePlatform : undefined
    res.reservationFor.departureTime = JsonLd.toDateTime(departureTime, 'hh:mm dd.M.yyyy', 'cz');

    res.reservationFor.arrivalBusStop.name = arrivalStation
    res.reservationFor.arrivalTime = JsonLd.toDateTime(arrivalTime, 'hh:mm dd.M.yyyy', 'cz');

    res.reservedTicket.ticketedSeat.seatNumber = seat
    res.reservedTicket.ticketToken = 'qrCode:' + node.childNodes[0].childNodes[0].content;

	ExtractorEngine.extractPrice(content.text, res);

    return res

}
