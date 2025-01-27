/*
 *   SPDX-FileCopyrightText: 2025 Grzegorz M
 *   SPDX-License-Identifier: LGPL-2.0-or-later
 */

function extractPdfTicket(pdf, node) {
    const dateRegex = /(?:Ważny|Ważny od) (\d\d.\d\d.\d\d\d\d)/
    const UICtableRegex = /\s*\*\s*\*\s+(\S.*?)\s{2,}(\S.*?)\s{2,}\*\s*\*\s+(\d)\n/;
    const routePlanRegex = /\s*(\d\d:\d\d) .* (\d\d:\d\d) .*/
    const trainRegex = /Pociąg REGIO nr (\d*)/
    const ownerRegex = /Właściciel: (\S.*?) \s/


    let reservations = [];

    // im not sure if each ticket is on one page, but UI looks like it could be
    pdf.pages.forEach((thisPage, index) => {
        const reservation = JsonLd.newTrainReservation();
        const thisTicket = thisPage.text;

        const thisDate = thisTicket.match(dateRegex)
        const UICtable = thisTicket.match(UICtableRegex);
        const routePlan = thisTicket.match(routePlanRegex);
        const train = thisTicket.match(trainRegex);
        const owner = thisTicket.match(ownerRegex);

        reservation.reservationFor.departureStation.name = UICtable[1];
        reservation.reservationFor.arrivalStation.name = UICtable[2];
        reservation.reservedTicket.ticketedSeat.seatingType = UICtable[3];
        reservation.reservedTicket.ticketedSeat.seatSection = "";
        reservation.reservedTicket.ticketedSeat.seatNumber = "";


        reservation.reservationFor.trainNumber = "PR "+train[1]; // This is Polregio only ticket
        reservation.reservationFor.departureTime = JsonLd.toDateTime(thisDate[1]+routePlan[1], "dd.MM.yyyyhh:mm", "pl");
        reservation.reservationFor.arrivalTime = JsonLd.toDateTime(thisDate[1]+routePlan[2], "dd.MM.yyyyhh:mm", "pl");

        reservation.underName.name = owner[1];

        reservation.reservedTicket.ticketToken = 'datamatrix:' +  node.childNodes[index].childNodes[0].content;


        reservations.push(reservation);
    });

    return reservations;
}
