/*
    SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "jsonld.h"

#include <QJsonObject>
#include <QString>

using namespace Qt::Literals::StringLiterals;

using namespace KItinerary;

QString JsonLd::typeName(const QJsonObject &obj)
{
    return normalizeTypeName(obj.value("@type"_L1).toString());
}

QString JsonLd::normalizeTypeName(QString &&typeName)
{
    // strip fully qualified names
    if (typeName.startsWith("http://schema.org/"_L1)) {
        typeName = typeName.mid(18);
    } else if (typeName.startsWith("https://schema.org/"_L1)) {
        typeName = typeName.mid(19);
    }
    return typeName;
}

bool JsonLd::isSchemaOrgNamespace(QStringView uri)
{
    return uri.startsWith("http://schema.org"_L1) || uri.startsWith("https://schema.org"_L1);
}
