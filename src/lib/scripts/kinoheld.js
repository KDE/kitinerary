/*
   SPDX-FileCopyrightText: 2023 Kai Uwe Broulik <kde@broulik.de>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

function main(pass, node)
{
    let res = node.result[0];

    // Override "Gleich startet Dein kinoheld Erlebnis" with actual cinema name.
    const cinemaAddress = pass.field["cinema-address"];
    if (cinemaAddress) {
        res.reservationFor.location.name = cinemaAddress.label;
    }

    const movieInfo = pass.field["movieinfo"];
    if (movieInfo) {
        res.reservationFor.description = movieInfo.value;
    }

    // Even with a single ticket, it's ticketnumber*s*.
    const ticketNumbers = pass.field["ticketnumbers"];
    if (ticketNumbers) {
        res.reservationNumber = ticketNumbers.value;
    }

    let seatLd = JsonLd.newObject("Seat");

    const seatNumbers = pass.field["seat-numbers"];
    if (seatNumbers) {
        const seats = seatNumbers.value.split("; ");
        let rows = [];
        let numbers = [];

        seats.forEach((seat) => {
            const seatParts = seat.split("/");

            rows.push(seatParts[0]);
            numbers.push(seatParts[1]);
        });

        // Itinerary doesn't support multiple seats on the same reservation,
        // concatenate them so the information isn't lost.
        // Except, if all seats are in the same row, show just a single row.
        if (new Set(rows).size === 1) {
            seatLd.seatRow = rows[0];
        } else {
            seatLd.seatRow = rows.join(", ");
        }
        seatLd.seatNumber = numbers.join(", ");
    }

    const seatCategory = pass.field["seat-cat"];
    if (seatCategory) {
        seatLd.seatingType = seatCategory.value;
    }

    res.reservedTicket.ticketedSeat = seatLd;

    const orderPage = pass.field["order-page"];
    if (orderPage) {
        res.potentialAction = JsonLd.newObject("UpdateAction");
        res.potentialAction.url = orderPage.value;
    }

    return res;
}
