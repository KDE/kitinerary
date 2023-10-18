/*
   SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
   SPDX-License-Identifier: LGPL-2.0-or-later
*/

function extractFullImage(image, node)
{
    return ExtractorEngine.extract(image, node.mimeType).result;
}

function promoteIcalEvent(event, node)
{
    let res = JsonLd.newEventReservation();
    res.reservationFor = node.result[0];
    return res;
}
