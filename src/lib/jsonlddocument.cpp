/*
   SPDX-FileCopyrightText: 2017 Volker Krause <vkrause@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "jsonlddocument.h"
#include "json/jsonld.h"
#include "json/jsonldimportfilter.h"
#include "logging.h"

#include <KItinerary/Action>
#include <KItinerary/BoatTrip>
#include <KItinerary/Brand>
#include <KItinerary/BusTrip>
#include <KItinerary/CreativeWork>
#include <KItinerary/Event>
#include <KItinerary/Flight>
#include <KItinerary/Organization>
#include <KItinerary/Person>
#include <KItinerary/Place>
#include <KItinerary/ProgramMembership>
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
#include <QSequentialIterable>

#include <cassert>
#include <cmath>
#include <cstring>

using namespace KItinerary;

struct TypeInfo {
    const char *name;
    const QMetaObject *mo;
    int metaTypeId;
};

static void registerBuiltInTypes(std::vector<TypeInfo> &r);
static std::vector<TypeInfo>& typeResgistry()
{
    static std::vector<TypeInfo> s_typeResgistry;
    if (s_typeResgistry.empty()) {
        registerBuiltInTypes(s_typeResgistry);
    }
    return s_typeResgistry;
}

template <typename T>
static void add(std::vector<TypeInfo> &registry)
{
    registry.push_back({ T::typeName(), &T::staticMetaObject, qMetaTypeId<T>() });
}

static void registerBuiltInTypes(std::vector<TypeInfo> &r)
{
    add<Action>(r);
    add<Airline>(r);
    add<Airport>(r);
    add<BoatReservation>(r);
    add<BoatTerminal>(r);
    add<BoatTrip>(r);
    add<Brand>(r);
    add<BusReservation>(r);
    add<BusStation>(r);
    add<BusTrip>(r);
    add<CancelAction>(r);
    add<CheckInAction>(r);
    add<CreativeWork>(r);
    add<DigitalDocument>(r);
    add<DownloadAction>(r);
    add<EmailMessage>(r);
    add<Event>(r);
    add<EventReservation>(r);
    add<Flight>(r);
    add<FlightReservation>(r);
    add<FoodEstablishment>(r);
    add<FoodEstablishmentReservation>(r);
    add<GeoCoordinates>(r);
    add<LocalBusiness>(r);
    add<LodgingBusiness>(r);
    add<LodgingReservation>(r);
    add<Organization>(r);
    add<Person>(r);
    add<Place>(r);
    add<PostalAddress>(r);
    add<ProgramMembership>(r);
    add<RentalCar>(r);
    add<RentalCarReservation>(r);
    add<ReserveAction>(r);
    add<Seat>(r);
    add<Taxi>(r);
    add<TaxiReservation>(r);
    add<Ticket>(r);
    add<TouristAttraction>(r);
    add<TouristAttractionVisit>(r);
    add<TrainReservation>(r);
    add<TrainStation>(r);
    add<TrainTrip>(r);
    add<UpdateAction>(r);
    add<ViewAction>(r);

    assert(std::is_sorted(r.begin(), r.end(), [](const auto &lhs, const auto &rhs) {
        return std::strcmp(lhs.name, rhs.name) < 0;
    }));
}

static QVariant createInstance(const QJsonObject &obj);
static QVariant createInstance(const QJsonObject &obj, const QMetaProperty &prop);

// Eurowings workarounds...
static const char* const fallbackDateTimePattern[] = {
    "yyyy-MM-dd HH:mm:ss",
    "yyyy-MM-dd HH:mm",
    "MM-dd-yyyy HH:mm", // yes, seriously ;(
    "yyyyMMddTHHmmsst"
};
static const auto fallbackDateTimePatternCount = sizeof(fallbackDateTimePattern) / sizeof(const char *);

static bool isEmptyJsonLdObject(const QJsonObject &obj)
{
    for (auto it = obj.begin(); it != obj.end(); ++it) {
        if (it.key() == QLatin1String("@type")) {
            continue;
        }
        if (it.value().type() != QJsonValue::Object) {
            return false;
        }
        if (!isEmptyJsonLdObject(it.value().toObject())) {
            return false;
        }
    }
    return true;
}

static QVariant propertyValue(const QMetaProperty &prop, const QJsonValue &v)
{
    // enum handling must be done first, as prop.type() == Int
    if (prop.isEnumType() && v.isString()) {
        auto value = v.toString();
        if (value.startsWith(QLatin1String("http://schema.org/"))) {
            value = value.mid(18);
        }
        const auto key = prop.enumerator().keyToValue(value.toUtf8().constData());
        return key;
    }

    switch (prop.userType()) {
    case QMetaType::QString:
        if (v.isDouble()) {
            double i = 0.0;
            const auto frac = std::modf(v.toDouble(), &i);
            if (frac == 0.0) {
                return QString::number(static_cast<uint64_t>(i));
            }
            return QString::number(v.toDouble());
        }
        return v.toString();
    case QMetaType::QDate:
        return QDate::fromString(v.toString(), Qt::ISODate);
    case QMetaType::QDateTime:
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
            if (dt.isNull()) {
                dt = QDateTime::fromString(QString(str).simplified().remove(QLatin1Char(' ')), Qt::ISODate);
            }
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
    case QMetaType::Double:
    case QMetaType::Float:
    {
        if (v.isDouble()) {
            return v.toDouble();
        }
        bool ok = false;
        const auto res = v.toString().toDouble(&ok);
        return ok ? res : QVariant();
    }
    case QMetaType::Int:
        if (v.isDouble()) {
            return v.toDouble();
        }
        return v.toString().toInt();
    case QMetaType::QTime:
        return QTime::fromString(v.toString(), Qt::ISODate);
    case QMetaType::QUrl:
        return QUrl(v.toString());
    case QMetaType::QVariantList:
    {
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
    default:
        break;
    }

    const auto obj = v.toObject();
    if (prop.userType() == qMetaTypeId<QVariant>() && isEmptyJsonLdObject(obj)) {
        return {};
    }
    return createInstance(obj, prop);
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
        if (!value.isNull()) {
            prop.writeOnGadget(v, value);
        }
    }
}

static QVariant createInstance(const QJsonObject& obj, const QString &type)
{
    const auto& registry = typeResgistry();
    const auto it = std::lower_bound(registry.begin(), registry.end(), type, [](const auto &lhs, const auto &rhs) {
        return QLatin1String(lhs.name) < rhs;
    });
    if (it != registry.end() && QLatin1String((*it).name) == type) {
        QVariant value(QMetaType((*it).metaTypeId), nullptr);
        createInstance((*it).mo, value.data(), obj);
        return value;
    }

    if (type == QLatin1String("QDateTime")) {
        auto dt = QDateTime::fromString(obj.value(QLatin1String("@value")).toString(), Qt::ISODate);
        dt.setTimeZone(QTimeZone(obj.value(QLatin1String("timezone")).toString().toUtf8()));
        return dt;
    }

    return {};
}

static QVariant createInstance(const QJsonObject &obj)
{
    return createInstance(obj, JsonLd::typeName(obj));
}

static QVariant createInstance(const QJsonObject &obj, const QMetaProperty &prop)
{
    const auto type = JsonLd::typeName(obj);
    if (!type.isEmpty()) {
        return createInstance(obj, type);
    }

    // if @type is (wrongly) not specified, try to recover from our own knowledge of a property type
    const auto mo = QMetaType(prop.userType()).metaObject();
    if (mo) {
        QVariant value(prop.metaType(), nullptr);
        createInstance(mo, value.data(), obj);
        return value;
    }

    return {};
}

QList<QVariant> JsonLdDocument::fromJson(const QJsonArray &array) {
    QList<QVariant> l;
    l.reserve(array.size());
    for (const auto &obj : array) {
        l.append(JsonLdDocument::fromJson(obj.toObject()));
    }
    return l;
}

QList<QVariant> JsonLdDocument::fromJson(const QJsonObject &obj) {
    const auto normalized = JsonLdImportFilter::filterObject(obj);
    QList<QVariant> result;
    result.reserve(normalized.size());
    for (const auto &n : normalized) {
        const auto v = createInstance(n.toObject());
        if (!v.isNull()) {
            result.push_back(v);
        }
    }
    return result;
}

QVariant JsonLdDocument::fromJsonSingular(const QJsonObject &obj)
{
    const auto normalized = JsonLdImportFilter::filterObject(obj);
    if (normalized.isEmpty()) {
        return {};
    }
    return createInstance(normalized.at(0).toObject());
}

bool JsonLd::valueIsNull(const QVariant &v)
{
    switch (v.userType()) {
        case QMetaType::QUrl:
            return !v.toUrl().isValid();
        case QMetaType::Float:
            return std::isnan(v.toFloat());
        case QMetaType::Double:
            return std::isnan(v.toDouble());
        case QMetaType::QVariantList:
            return v.toList().isEmpty();
        // starting with Qt6, QVariant::isNull is "shallow" and would miss the following as well
        case QMetaType::QString:
            return v.toString().isNull();
        case QMetaType::QDateTime:
            return v.toDateTime().isNull();
        case QMetaType::QDate:
            return v.toDate().isNull();
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

QJsonValue JsonLdDocument::toJsonValue(const QVariant &v)
{
    const auto mo = QMetaType(v.userType()).metaObject();
    if (!mo) {
        // basic types
        switch (v.userType()) {
        case QMetaType::QString:
            return v.toString();
        case QMetaType::Double:
            return v.toDouble();
        case QMetaType::Int:
            return v.toInt();
        case QMetaType::QDate:
            return v.toDate().toString(Qt::ISODate);
        case QMetaType::QDateTime:
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
        case QMetaType::QTime:
            return v.toTime().toString(Qt::ISODate);
        case QMetaType::QUrl:
            return v.toUrl().toString();
        case QMetaType::Bool:
            return v.toBool();
        case QMetaType::Float:
            return v.toFloat();
        default:
            break;
        }

        if (v.canConvert<QVariantList>()) {
            auto iterable = v.value<QSequentialIterable>();
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
            auto value = QString::fromUtf8(prop.enumerator().valueToKey(key));
            // this is (ab)used elsewhere, so let's not interfere with enum serialization there for now
            if (strncmp(mo->className(), "KItinerary::", 12) == 0) {
                value = QLatin1String("http://schema.org/") + value;
            }
            obj.insert(QString::fromUtf8(prop.name()), value);
            continue;
        } else if (QMetaType(prop.userType()).flags() & QMetaType::IsEnumeration) { // external enums
            obj.insert(QString::fromUtf8(prop.name()), prop.readOnGadget(v.constData()).toString());
            continue;
        }

        const auto value = prop.readOnGadget(v.constData());
        if (!JsonLd::valueIsNull(value)) {
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

QJsonArray JsonLdDocument::toJson(const QList<QVariant> &data) {
    QJsonArray a;
    for (const auto &d : data) {
        const auto value = toJsonValue(d);
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
    const auto value = toJsonValue(data);
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

        if (prop.isEnumType() && rhs.userType() == QMetaType::QString) { // internal enums in this QMO
            const auto key = prop.enumerator().keyToValue(rhs.toString().toUtf8().constData());
            prop.writeOnGadget(res.data(), key);
            continue;
        }
        if ((QMetaType(prop.userType()).flags() & QMetaType::IsEnumeration) && rhs.userType() == QMetaType::QString) { // external enums
            const QMetaType mt(prop.userType());
            const auto mo = mt.metaObject();
            if (!mo) {
                qCWarning(Log) << "No meta object found for enum type:" << prop.typeName();
                continue;
            }
            const auto enumIdx = mo->indexOfEnumerator(prop.typeName() + strlen(mo->className()) + 2);
            if (enumIdx < 0) {
                qCWarning(Log) << "Could not find QMetaEnum for" << prop.typeName();
                continue;
            }
            const auto me = mo->enumerator(enumIdx);
            bool success = false;
            const auto numValue = me.keyToValue(rhs.toString().toUtf8().constData(), &success);
            if (!success) {
                qCWarning(Log) << "Unknown enum value" << rhs.toString() << "for" << prop.typeName();
                continue;
            }
            auto valueData = mt.create();
            *reinterpret_cast<int*>(valueData) = numValue;
            QVariant value(prop.metaType(), valueData);
            prop.writeOnGadget(res.data(), value);
            continue;
        }

        auto pv = prop.readOnGadget(rhs.constData());
        if ((QMetaType(pv.userType()).flags() & QMetaType::IsGadget) && QMetaType(pv.userType()).metaObject()) {
            pv = apply(prop.readOnGadget(lhs.constData()), pv);
        }
        if (!JsonLd::valueIsNull(pv)) {
            prop.writeOnGadget(res.data(), pv);
        }
    }

    return res;
}

void JsonLdDocument::registerType(const char *typeName, const QMetaObject *mo, int metaTypeId)
{
    auto &registry = typeResgistry();
    const auto it = std::lower_bound(registry.begin(), registry.end(), typeName, [](const auto &lhs, const auto *rhs) {
        return std::strcmp(lhs.name, rhs) < 0;
    });
    if (it != registry.end() && std::strcmp((*it).name, typeName) == 0) {
        qCWarning(Log) << "Type already registered:" << typeName;
    } else {
        registry.insert(it, { typeName, mo, metaTypeId });
    }
}
