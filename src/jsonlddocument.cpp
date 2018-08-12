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
#include "jsonldimportfilter.h"
#include "logging.h"

#include <KItinerary/Action>
#include <KItinerary/BusTrip>
#include <KItinerary/Event>
#include <KItinerary/Flight>
#include <KItinerary/Organization>
#include <KItinerary/Person>
#include <KItinerary/Place>
#include <KItinerary/Reservation>
#include <KItinerary/RentalCar>
#include <KItinerary/Taxi>
#include <KItinerary/Ticket>
#include <KItinerary/TrainTrip>
#include <KItinerary/Visit>

#include <QDateTime>
#include <QJsonArray>
#include <QJsonObject>
#include <QMetaProperty>
#include <QUrl>
#include <QTimeZone>
#include <QVector>

#include <cmath>

using namespace KItinerary;

static QVariant createInstance(const QJsonObject &obj);

// Eurowings workarounds...
static const char *fallbackDateTimePattern[] = {
    "yyyy-MM-dd HH:mm:ss",
    "yyyy-MM-dd HH:mm",
    "MM-dd-yyyy HH:mm" // yes, seriously ;(
};
static const auto fallbackDateTimePatternCount = sizeof(fallbackDateTimePattern) / sizeof(const char *);

static double doubleValue(const QJsonValue &v)
{
    if (v.isDouble()) {
        return v.toDouble();
    }
    return v.toString().toDouble();
}

static QVariant propertyValue(const QMetaProperty &prop, const QJsonValue &v)
{
    switch (prop.type()) {
    case QVariant::String:
        return v.toString();
    case QVariant::Date:
        return QDate::fromString(v.toString(), Qt::ISODate);
    case QVariant::DateTime:
    {
        QDateTime dt;
        if (v.isObject()) {
            const auto dtObj = v.toObject();
            if (dtObj.value(QLatin1String("@type")).toString() == QLatin1String("QDateTime")) {
                dt = QDateTime::fromString(dtObj.value(QLatin1String("@value")).toString(), Qt::ISODate);
                dt.setTimeZone(QTimeZone(dtObj.value(QLatin1String("timezone")).toString().toUtf8()));
            }
        } else {
            auto str = v.toString();
            dt = QDateTime::fromString(str, Qt::ISODate);
            for (unsigned int i = 0; i < fallbackDateTimePatternCount && dt.isNull(); ++i) {
                dt = QDateTime::fromString(str, QString::fromLatin1(fallbackDateTimePattern[i]));
            }
            if (dt.isNull()) {
                qCDebug(Log) << "Datetime parsing failed for" << str;
            }
        }

        return dt;
    }
    case QVariant::Double:
        return doubleValue(v);
    case QVariant::Int:
        if (v.isDouble())
            return v.toDouble();
        return v.toString().toInt();
    case QVariant::Url:
        return QUrl(v.toString());
    default:
        break;
    }
    if (prop.type() == qMetaTypeId<float>()) {
        return doubleValue(v);
    }

    if (prop.userType() == qMetaTypeId<QVariantList>()) {
        QVariantList l;
        if (v.isArray()) {
            const auto array = v.toArray();
            l.reserve(array.size());
            for (const auto &elem : array) {
                const auto var = createInstance(elem.toObject());
                if (!var.isNull()) {
                    l.push_back(var);
                }
            }
        }
        return QVariant::fromValue(l);
    }

    return createInstance(v.toObject());
}

static void createInstance(const QMetaObject *mo, void *v, const QJsonObject &obj)
{
   for (auto it = obj.begin(); it != obj.end(); ++it) {
        if (it.key().startsWith(QLatin1Char('@'))) {
            continue;
        }
        const auto idx = mo->indexOfProperty(it.key().toLatin1().constData());
        if (idx < 0) {
            qCDebug(Log) << "property" << it.key() << "could not be set on object of type" << mo->className();
            continue;
        }
        const auto prop = mo->property(idx);
        const auto value = propertyValue(prop, it.value());
        prop.writeOnGadget(v, value);
    }
}

template<typename T>
static QVariant createInstance(const QJsonObject &obj)
{
    T t;
    createInstance(&T::staticMetaObject, &t, obj);
    return QVariant::fromValue(t);
}

#define MAKE_FACTORY(Class) \
    if (type == QLatin1String(#Class)) \
        return createInstance<Class>(obj)

static QVariant createInstance(const QJsonObject &obj)
{
    const auto type = obj.value(QLatin1String("@type")).toString();
    MAKE_FACTORY(Action);
    MAKE_FACTORY(Airline);
    MAKE_FACTORY(Airport);
    MAKE_FACTORY(BusReservation);
    MAKE_FACTORY(BusStation);
    MAKE_FACTORY(BusTrip);
    MAKE_FACTORY(CancelAction);
    MAKE_FACTORY(CheckInAction);
    MAKE_FACTORY(DownloadAction);
    MAKE_FACTORY(Event);
    MAKE_FACTORY(EventReservation);
    MAKE_FACTORY(Flight);
    MAKE_FACTORY(FlightReservation);
    MAKE_FACTORY(FoodEstablishment);
    MAKE_FACTORY(FoodEstablishmentReservation);
    MAKE_FACTORY(RentalCarReservation);
    MAKE_FACTORY(RentalCar);
    MAKE_FACTORY(GeoCoordinates);
    MAKE_FACTORY(LodgingBusiness);
    MAKE_FACTORY(LodgingReservation);
    MAKE_FACTORY(Organization);
    MAKE_FACTORY(Person);
    MAKE_FACTORY(Place);
    MAKE_FACTORY(PostalAddress);
    MAKE_FACTORY(Seat);
    MAKE_FACTORY(TaxiReservation);
    MAKE_FACTORY(Taxi);
    MAKE_FACTORY(Ticket);
    MAKE_FACTORY(TouristAttraction);
    MAKE_FACTORY(TouristAttractionVisit);
    MAKE_FACTORY(TrainReservation);
    MAKE_FACTORY(TrainStation);
    MAKE_FACTORY(TrainTrip);
    MAKE_FACTORY(UpdateAction);
    MAKE_FACTORY(ViewAction);    
    return {};
}

#undef MAKE_FACTORY

QVector<QVariant> JsonLdDocument::fromJson(const QJsonArray &array)
{
    QVector<QVariant> l;
    l.reserve(array.size());
    for (const auto &obj : array) {
        const auto v = createInstance(JsonLdImportFilter::filterObject(obj.toObject()));
        if (!v.isNull()) {
            l.push_back(v);
        }
    }
    return l;
}

static bool valueIsNull(const QVariant &v)
{
    if (v.type() == QVariant::Url) {
        return !v.toUrl().isValid();
    }
    if (v.type() == qMetaTypeId<float>()) {
        return std::isnan(v.toFloat());
    }
    return v.isNull();
}

static QJsonValue toJson(const QVariant &v)
{
    const auto mo = QMetaType(v.userType()).metaObject();
    if (!mo) {
        // basic types
        switch (v.type()) {
        case QVariant::String:
            return v.toString();
        case QVariant::Double:
            return v.toDouble();
        case QVariant::Int:
            return v.toInt();
        case QVariant::Date:
            return v.toDate().toString(Qt::ISODate);
        case QVariant::DateTime:
        {
            const auto dt = v.toDateTime();
            if (dt.timeSpec() == Qt::TimeZone) {
                QJsonObject dtObj;
                dtObj.insert(QStringLiteral("@type"), QStringLiteral("QDateTime"));
                dtObj.insert(QStringLiteral("@value"), dt.toString(Qt::ISODate));
                dtObj.insert(QStringLiteral("timezone"), QString::fromUtf8(dt.timeZone().id()));
                return dtObj;
            }
            return v.toDateTime().toString(Qt::ISODate);
        }
        case QVariant::Url:
            return v.toUrl().toString();
        default:
            break;
        }
        if (v.userType() == qMetaTypeId<float>()) {
            return v.toFloat();
        }

        if (v.canConvert<QVariantList>()) {
            QSequentialIterable iterable = v.value<QSequentialIterable>();
            if (iterable.size() == 0) {
                return {};
            }
            QJsonArray array;
            for (const auto &var : iterable) {
                array.push_back(toJson(var));
            }
            return array;
        }

        qCDebug(Log) << "unhandled value:" << v;
        return {};
    }

    // composite types
    QJsonObject obj;
    obj.insert(QStringLiteral("@type"), JsonLdDocument::readProperty(v, "className").toString());
    for (int i = 0; i < mo->propertyCount(); ++i) {
        const auto prop = mo->property(i);
        if (!prop.isStored()) {
            continue;
        }
        const auto value = prop.readOnGadget(v.constData());
        if (!valueIsNull(value)) {
            const auto jsVal = toJson(value);
            if (jsVal.type() != QJsonValue::Null) {
                obj.insert(QString::fromUtf8(prop.name()), jsVal);
            }
        }
    }
    if (obj.size() > 1) {
        return obj;
    }

    return {};
}

QJsonArray JsonLdDocument::toJson(const QVector<QVariant> &data)
{
    QJsonArray a;
    for (const auto &d : data) {
        const auto value = ::toJson(d);
        if (!value.isObject()) {
            continue;
        }
        auto obj = value.toObject();
        obj.insert(QStringLiteral("@context"), QStringLiteral("http://schema.org"));
        a.push_back(obj);
    }
    return a;
}

QVariant JsonLdDocument::readProperty(const QVariant &obj, const char *name)
{
    const auto mo = QMetaType(obj.userType()).metaObject();
    if (!mo) {
        return {};
    }

    const auto idx = mo->indexOfProperty(name);
    if (idx < 0) {
        return {};
    }

    const auto prop = mo->property(idx);
    return prop.readOnGadget(obj.constData());
}

void JsonLdDocument::writeProperty(QVariant &obj, const char *name, const QVariant &value)
{
    const auto mo = QMetaType(obj.userType()).metaObject();
    if (!mo) {
        return;
    }

    writePropertyImpl(mo, obj.data(), name, value);
}

void JsonLdDocument::writePropertyImpl(const QMetaObject* mo, void* obj, const char* name, const QVariant& value)
{
    const auto idx = mo->indexOfProperty(name);
    if (idx < 0) {
        return;
    }

    const auto prop = mo->property(idx);
    prop.writeOnGadget(obj, value);
}

void JsonLdDocument::removeProperty(QVariant &obj, const char *name)
{
    writeProperty(obj, name, QVariant());
}

QVariant JsonLdDocument::apply(const QVariant& lhs, const QVariant& rhs)
{
    if (rhs.isNull()) {
        return lhs;
    }
    if (lhs.isNull()) {
        return rhs;
    }
    if (lhs.userType() != rhs.userType()) {
        qCWarning(Log) << "type mismatch during merging:" << lhs << rhs;
        return {};
    }

    auto res = lhs;
    const auto mo = QMetaType(res.userType()).metaObject();
    for (int i = 0; i < mo->propertyCount(); ++i) {
        const auto prop = mo->property(i);
        if (!prop.isStored()) {
            continue;
        }
        auto pv = prop.readOnGadget(rhs.constData());
        if (QMetaType(pv.userType()).metaObject()) {
            pv = apply(prop.readOnGadget(lhs.constData()), pv);
        }
        if (!pv.isNull()) {
            prop.writeOnGadget(res.data(), pv);
        }
    }

    return res;
}
