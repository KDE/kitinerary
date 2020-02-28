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
#include <KItinerary/Brand>
#include <KItinerary/BusTrip>
#include <KItinerary/CreativeWork>
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
static const char* const fallbackDateTimePattern[] = {
    "yyyy-MM-dd HH:mm:ss",
    "yyyy-MM-dd HH:mm",
    "MM-dd-yyyy HH:mm", // yes, seriously ;(
    "yyyyMMddTHHmmsst"
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
            // HACK QDateTimeParser handles 't' in the format but then forces it back to LocalTime in the end...
            if (dt.isValid() && dt.timeSpec() == Qt::LocalTime && str.endsWith(QLatin1Char('Z'))) {
                dt = dt.toTimeSpec(Qt::UTC);
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
        if (v.isDouble()) {
            return v.toDouble();
        }
        return v.toString().toInt();
    case QVariant::Time:
        return QTime::fromString(v.toString(), Qt::ISODate);
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
                if (elem.isObject()) {
                    const auto var = createInstance(elem.toObject());
                    if (!var.isNull()) {
                        l.push_back(var);
                    }
                } else if (elem.isString()) {
                    l.push_back(elem.toString());
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
    MAKE_FACTORY(Brand);
    MAKE_FACTORY(BusReservation);
    MAKE_FACTORY(BusStation);
    MAKE_FACTORY(BusTrip);
    MAKE_FACTORY(CancelAction);
    MAKE_FACTORY(CheckInAction);
    MAKE_FACTORY(CreativeWork);
    MAKE_FACTORY(DigitalDocument);
    MAKE_FACTORY(DownloadAction);
    MAKE_FACTORY(EmailMessage);
    MAKE_FACTORY(Event);
    MAKE_FACTORY(EventReservation);
    MAKE_FACTORY(Flight);
    MAKE_FACTORY(FlightReservation);
    MAKE_FACTORY(FoodEstablishment);
    MAKE_FACTORY(FoodEstablishmentReservation);
    MAKE_FACTORY(LocalBusiness);
    MAKE_FACTORY(RentalCarReservation);
    MAKE_FACTORY(RentalCar);
    MAKE_FACTORY(ReserveAction);
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

    if (type == QLatin1String("QDateTime")) {
        auto dt = QDateTime::fromString(obj.value(QLatin1String("@value")).toString(), Qt::ISODate);
        dt.setTimeZone(QTimeZone(obj.value(QLatin1String("timezone")).toString().toUtf8()));
        return dt;
    }

    return {};
}
#undef MAKE_FACTORY

static QVector<QVariant> fromJson(const QJsonObject &obj) // TODO this should replace JsonLdDocument::fromJson once we can change the API
{
    const auto normalized = JsonLdImportFilter::filterObject(obj);
    QVector<QVariant> result;
    result.reserve(normalized.size());
    for (const auto &n : normalized) {
        const auto v = createInstance(n.toObject());
        if (!v.isNull()) {
            result.push_back(v);
        }
    }
    return result;
}

QVector<QVariant> JsonLdDocument::fromJson(const QJsonArray &array)
{
    QVector<QVariant> l;
    l.reserve(array.size());
    for (const auto &obj : array) {
        l.append(::fromJson(obj.toObject()));
    }
    return l;
}

QVariant JsonLdDocument::fromJson(const QJsonObject& obj)
{
    return fromJsonSingular(obj); // ### temporary, see above
}

QVariant JsonLdDocument::fromJsonSingular(const QJsonObject &obj)
{
    const auto normalized = JsonLdImportFilter::filterObject(obj);
    if (normalized.isEmpty()) {
        return {};
    }
    return createInstance(normalized.at(0).toObject());
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

static QString typeName(const QMetaObject *mo, const QVariant &v)
{
    const auto n = JsonLdDocument::readProperty(v, "className").toString();
    if (!n.isEmpty()) {
        return n;
    }

    if (auto c = strstr(mo->className(), "::")) {
        return QString::fromUtf8(c + 2);
    }
    return QString::fromUtf8(mo->className());
}

static QJsonValue toJsonValue(const QVariant &v)
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
        case QVariant::Time:
            return v.toTime().toString(Qt::ISODate);
        case QVariant::Url:
            return v.toUrl().toString();
        case QVariant::Bool:
            return v.toBool();
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
                array.push_back(toJsonValue(var));
            }
            return array;
        }

        qCDebug(Log) << "unhandled value:" << v;
        return {};
    }

    // composite types
    QJsonObject obj;
    obj.insert(QStringLiteral("@type"), typeName(mo, v));
    for (int i = 0; i < mo->propertyCount(); ++i) {
        const auto prop = mo->property(i);
        if (!prop.isStored()) {
            continue;
        }

        if (prop.isEnumType()) { // enums defined in this QMO
            const auto key = prop.readOnGadget(v.constData()).toInt();
            const auto value = prop.enumerator().valueToKey(key);
            obj.insert(QString::fromUtf8(prop.name()), QString::fromUtf8(value));
            continue;
        } else if (QMetaType::typeFlags(prop.userType()) & QMetaType::IsEnumeration) { // external enums
            obj.insert(QString::fromUtf8(prop.name()), prop.readOnGadget(v.constData()).toString());
            continue;
        }

        const auto value = prop.readOnGadget(v.constData());
        if (!valueIsNull(value)) {
            const auto jsVal = toJsonValue(value);
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
        const auto value = ::toJsonValue(d);
        if (!value.isObject()) {
            continue;
        }
        auto obj = value.toObject();
        obj.insert(QStringLiteral("@context"), QStringLiteral("http://schema.org"));
        a.push_back(obj);
    }
    return a;
}

QJsonObject JsonLdDocument::toJson(const QVariant& data)
{
    const auto value = ::toJsonValue(data);
    if (!value.isObject()) {
        return {};
    }
    auto obj = value.toObject();
    obj.insert(QStringLiteral("@context"), QStringLiteral("http://schema.org"));
    return obj;
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

        if (prop.isEnumType() && rhs.type() == QVariant::String) { // internal enums in this QMO
            const auto key = prop.enumerator().keyToValue(rhs.toString().toUtf8().constData());
            prop.writeOnGadget(res.data(), key);
            continue;
        }
        if ((QMetaType::typeFlags(prop.userType()) & QMetaType::IsEnumeration) && rhs.type() == QVariant::String) { // external enums
            const QMetaType mt(prop.userType());
            const auto mo = mt.metaObject();
            if (!mo) {
                qCWarning(Log) << "No meta object found for enum type:" << prop.type();
                continue;
            }
            const auto enumIdx = mo->indexOfEnumerator(prop.typeName() + strlen(mo->className()) + 2);
            if (enumIdx < 0) {
                qCWarning(Log) << "Could not find QMetaEnum for" << prop.type();
                continue;
            }
            const auto me = mo->enumerator(enumIdx);
            bool success = false;
            const auto numValue = me.keyToValue(rhs.toString().toUtf8().constData(), &success);
            if (!success) {
                qCWarning(Log) << "Unknown enum value" << rhs.toString() << "for" << prop.type();
                continue;
            }
            auto valueData = mt.create();
            *reinterpret_cast<int*>(valueData) = numValue;
            QVariant value(prop.userType(), valueData);
            prop.writeOnGadget(res.data(), value);
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
