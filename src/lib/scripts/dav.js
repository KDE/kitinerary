/*
   SPDX-FileCopyrightText: 2025 Stephan Olbrich <stephanolbrich@gmx.de>
   SPDX-License-Identifier: LGPL-2.0-or-later
*/

function parseMembershipCard(pdf, node) {
    const c = pdf.text.split(/\n+/)
    const num = c[6].match(/\d{3}\/\d{2}\/(\d+)\*.*$/)[1]
    let card = {
        '@type': 'ProgramMembership',
        programName: 'DAV Mitgliedsausweis (' + c[3].match(/^ *(\S Mitglied)$/)[1] + ')',
        membershipNumber: num,
        member: {
            '@type': 'Person',
            name: c[4],
        },
        token: 'barcode128:' + node.childNodes[0].childNodes[0].content,
    };
    return card;
}
