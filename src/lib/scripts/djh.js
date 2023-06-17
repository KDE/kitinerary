/*
   SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
   SPDX-License-Identifier: LGPL-2.0-or-later
*/

function parseMembershipCard(code) {
    console.log(code);
    const c = code.split(/;/);
    let card = {
        '@type': 'ProgramMembership',
        programName: 'DJH Mitgliedskarte (' + c[10] + ')',
        membershipNumber: c[0].substr(1),
        member: {
            '@type': 'Person',
            givenName: c[2],
            familyName: c[3],
        },
        token: 'dataMatrix:' + code,
        validFrom: JsonLd.toDateTime(c[12], 'dd.MM.yyyy?', 'de'),
        validUntil: JsonLd.toDateTime(c[11], 'MM/yyyy', 'de'),
    };
    return card;
}
