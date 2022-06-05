/*
   SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
   SPDX-License-Identifier: LGPL-2.0-or-later
*/

function parsePdfTicket(pdf, node, trigger) {
    let ticket = trigger.result[0];
    const text = pdf.pages[trigger.location].text;
    ticket.name = text.match(/^(.*)\n/)[1];
    return ticket;
}
