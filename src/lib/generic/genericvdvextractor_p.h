/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KITINERARY_GENERICVDVEXTRACTOR_P_H
#define KITINERARY_GENERICVDVEXTRACTOR_P_H

class QByteArray;
class QJsonArray;

namespace KItinerary {

/** Generic extractor for VDV tickets. */
namespace GenericVdvExtractor
{
    QJsonArray extract(const QByteArray &data);
}

}

#endif // KITINERARY_GENERICVDVEXTRACTOR_P_H
