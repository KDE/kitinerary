/*
    SPDX-FileCopyrightText: 2018-2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

class QByteArray;
class QJsonArray;

namespace KItinerary {

/** Generic UIC 918 ticket code extractor. */
namespace GenericUic918Extractor
{
    void extract(const QByteArray &data, QJsonArray &result);
}

}

