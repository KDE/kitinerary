/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

class QByteArray;
class QJsonArray;

namespace KItinerary {

/** Generic extractor for VDV tickets. */
namespace GenericVdvExtractor
{
    QJsonArray extract(const QByteArray &data);
}

}

