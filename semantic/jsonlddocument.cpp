/*
   Copyright (c) 2017 Volker Krause <vkrause@kde.org>

   This library is free software; you can redistribute it and/or modify it
   under the terms of the GNU Library General Public License as published by
   the Free Software Foundation; either version 2 of the License, or (at your
   option) any later version.

   This library is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
   FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
   License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to the
   Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
   02110-1301, USA.
*/

#include "jsonlddocument.h"
#include "datatypes.h"
#include "semantic_debug.h"

#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMetaProperty>

static QVariant createInstance(const QJsonObject &obj);

// Eurowings workarounds...
static const char* fallbackDateTimePattern[] = {
    "yyyy-MM-dd HH:mm:ss",
    "yyyy-MM-dd HH:mm",
    "MM-dd-yyyy HH:mm" // yes, seriously ;(
};
static const auto fallbackDateTimePatternCount = sizeof(fallbackDateTimePattern) / sizeof(const char*);

static QVariant propertyValue(const QMetaProperty &prop, const QJsonValue &v)
{
    switch (prop.type()) {
        case QVariant::String:
            return v.toString();
        case QVariant::DateTime:
        {
            auto str = v.toString();
            auto dt = QDateTime::fromString(str, Qt::ISODate);
            for (unsigned int i = 0; i < fallbackDateTimePatternCount && dt.isNull(); ++i) {
                dt = QDateTime::fromString(str, QString::fromLatin1(fallbackDateTimePattern[i]));
            }
            if (dt.isNull())
                qCDebug(SEMANTIC_LOG) << "Datetime parsing failed for" << str;
            return dt;
        }
        default:
            break;
    }
    return createInstance(v.toObject());
}

template <typename T>
static QVariant createInstance(const QJsonObject &obj)
{
    T t;
    for (auto it = obj.begin(); it != obj.end(); ++it) {
        if (it.key().startsWith(QLatin1Char('@')))
            continue;
        const auto idx = T::staticMetaObject.indexOfProperty(it.key().toLatin1());
        if (idx < 0) {
            qCDebug(SEMANTIC_LOG) << "property" << it.key() << "could not be set on object of type" << T::staticMetaObject.className();
            continue;
        }
        const auto prop = T::staticMetaObject.property(idx);
        const auto value = propertyValue(prop, it.value());
        prop.writeOnGadget(&t, value);
    }
    return QVariant::fromValue(t);
}

#define MAKE_FACTORY(Class) \
    if (type == QLatin1String(#Class)) \
        return createInstance<Class>(obj)

static QVariant createInstance(const QJsonObject &obj)
{
    const auto type = obj.value(QLatin1String("@type")).toString();
    MAKE_FACTORY(Airline);
    MAKE_FACTORY(Airport);
    MAKE_FACTORY(FlightReservation);
    MAKE_FACTORY(Flight);
    MAKE_FACTORY(LodgingBusiness);
    MAKE_FACTORY(LodgingReservation);
    MAKE_FACTORY(PostalAddress);
    return {};
}

#undef MAKE_FACTORY

QVariantList JsonLdDocument::fromJson(const QJsonArray &array)
{
    QVariantList l;
    for (const auto &obj : array) {
        const auto v = createInstance(obj.toObject());
        if (!v.isNull())
            l.push_back(v);
    }    return l;
}
