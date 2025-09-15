// SPDX-FileCopyrightText: ⓒ 2025 Volker Krause <vkrause@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

function extractPass(pass, node) {
    let res = node.result[0];
    res.reservationFor.location.name = pass.field['venueName'].value;
    res.reservedTicket.name = pass.field['ticketTypeName'].value;
    res.reservedTicket.ticketedSeat = {
        '@type': 'Seat',
        seatSection: pass.field['sectionName'].value,
        seatRow: pass.field['rowName'].value,
        seatNumber: pass.field['seatName'].value
    };
    res.reservationNumber = pass.field['ticketNoBack'].value;
    return res;
}
