/*
 * SPDX-FileCopyrightText: 2026 Kai Uwe Broulik <kde@broulik.de>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
*/

function main(pass, node) {
    let res = node.result[0];

    const boardingZoneField = pass.field["boardingZone"];
    if (boardingZoneField) {
        res.boardingGroup = boardingZoneField.value;
    }

    if (res.reservationFor.departureTerminal) {
        const departureIata = res.reservationFor.departureAirport.iataCode;
        // Condor puts airport code in terminal string, e.g. "STR | 3"
        res.reservationFor.departureTerminal = res.reservationFor.departureTerminal.replace(new RegExp("^" + departureIata + " \\| "), "");
    }

    return res;
}
