/*
    SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "jsonld.h"

#include <QJsonObject>

using namespace KItinerary;

QString JsonLd::typeName(const QJsonObject &obj)
{
    QString n = obj.value(QLatin1String("@type")).toString();
    if (n.startsWith(QLatin1String("http://schema.org/"))) { // strip fully qualified names
        n = n.mid(18);
    }
    return n;
}
