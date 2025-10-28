// SPDX-FileCopyrightText: 2025 Volker Krause <vkrause@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

function extractBarcode(uic, node) {
    let ticket = node.result[0];
    ticket.name = uic.ticketLayout.text(1, 0, 53 ,1).trim();
    const times = uic.block("3306FI").contentText.match(/(\d{2}.\d{2}\.\d{4} \d\d:\d\d)(\d{2}.\d{2}\.\d{4} \d\d:\d\d)/);
    ticket.validFrom = JsonLd.toDateTime(times[1], "dd.MM.yyyy HH:mm", "de");
    ticket.validUntil = JsonLd.toDateTime(times[2], "dd.MM.yyyy HH:mm", "de");
    const name = uic.block("3306VD").contentText.match(/\|(.*)\|\|(.*)\|/);
    ticket.underName = {
        '@type': 'Person',
        givenName: name[1],
        familyName: name[2]
    };
    return ticket;
}
