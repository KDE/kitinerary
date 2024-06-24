/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "jsonld.h"
#include "reservationconverter.h"

#include <KItinerary/JsonLdDocument>

#include <QDebug>
#include <QJSEngine>
#include <QJsonArray>
#include <QJsonObject>
#include <QLocale>
#include <QMetaProperty>
#include <QRegularExpression>
#include <QUrl>

using namespace KItinerary;

JsApi::JsonLd::JsonLd(QJSEngine* engine)
    : QObject(engine)
    , m_engine(engine)
{
}

JsApi::JsonLd::~JsonLd() = default;

QJSValue JsApi::JsonLd::newObject(const QString &typeName) const
{
    auto v = m_engine->newObject();
    v.setProperty(QStringLiteral("@type"), typeName);
    return v;
}

QJSValue JsApi::JsonLd::newPlace(const QString &type) const
{
    const auto addr = newObject(QStringLiteral("PostalAddress"));
    const auto geo = newObject(QStringLiteral("GeoCoordinates"));

    auto p = newObject(type);
    p.setProperty(QStringLiteral("address"), addr);
    p.setProperty(QStringLiteral("geo"), geo);

    return p;
}

QJSValue JsApi::JsonLd::newFlightReservation() const
{
    const auto dep = newPlace(QStringLiteral("Airport"));
    const auto arr = newPlace(QStringLiteral("Airport"));
    const auto airline = newObject(QStringLiteral("Airline"));
    const auto person = newObject(QStringLiteral("Person"));

    auto resFor = newObject(QStringLiteral("Flight"));
    resFor.setProperty(QStringLiteral("departureAirport"), dep);
    resFor.setProperty(QStringLiteral("arrivalAirport"), arr);
    resFor.setProperty(QStringLiteral("airline"), airline);

    const auto ticket = newObject(QStringLiteral("Ticket"));

    auto res = newObject(QStringLiteral("FlightReservation"));
    res.setProperty(QStringLiteral("reservationFor"), resFor);
    res.setProperty(QStringLiteral("underName"), person);
    res.setProperty(QStringLiteral("reservedTicket"), ticket);

    return res;
}

QJSValue JsApi::JsonLd::newTrainReservation() const
{
    const auto dep = newPlace(QStringLiteral("TrainStation"));
    const auto arr = newPlace(QStringLiteral("TrainStation"));
    const auto person = newObject(QStringLiteral("Person"));
    const auto seat = newObject(QStringLiteral("Seat"));

    auto ticket = newObject(QStringLiteral("Ticket"));
    ticket.setProperty(QStringLiteral("ticketedSeat"), seat);

    auto resFor = newObject(QStringLiteral("TrainTrip"));
    resFor.setProperty(QStringLiteral("departureStation"), dep);
    resFor.setProperty(QStringLiteral("arrivalStation"), arr);
    resFor.setProperty(QStringLiteral("provider"), newObject(QStringLiteral("Organization")));

    auto res = newObject(QStringLiteral("TrainReservation"));
    res.setProperty(QStringLiteral("reservationFor"), resFor);
    res.setProperty(QStringLiteral("underName"), person);
    res.setProperty(QStringLiteral("reservedTicket"), ticket);
    res.setProperty(QStringLiteral("programMembershipUsed"), newObject(QStringLiteral("ProgramMembership")));

    return res;
}

QJSValue JsApi::JsonLd::newBusReservation() const
{
    const auto dep = newPlace(QStringLiteral("BusStation"));
    const auto arr = newPlace(QStringLiteral("BusStation"));
    const auto person = newObject(QStringLiteral("Person"));
    const auto seat = newObject(QStringLiteral("Seat"));

    auto ticket = newObject(QStringLiteral("Ticket"));
    ticket.setProperty(QStringLiteral("ticketedSeat"), seat);

    auto resFor = newObject(QStringLiteral("BusTrip"));
    resFor.setProperty(QStringLiteral("departureBusStop"), dep);
    resFor.setProperty(QStringLiteral("arrivalBusStop"), arr);

    auto res = newObject(QStringLiteral("BusReservation"));
    res.setProperty(QStringLiteral("reservationFor"), resFor);
    res.setProperty(QStringLiteral("underName"), person);
    res.setProperty(QStringLiteral("reservedTicket"), ticket);

    return res;
}

QJSValue JsApi::JsonLd::newLodgingReservation() const
{
    const auto person = newObject(QStringLiteral("Person"));
    const auto resFor = newPlace(QStringLiteral("LodgingBusiness"));

    auto res = newObject(QStringLiteral("LodgingReservation"));
    res.setProperty(QStringLiteral("reservationFor"), resFor);
    res.setProperty(QStringLiteral("underName"), person);

    return res;
}

QJSValue JsApi::JsonLd::newEventReservation() const
{
    auto resFor = newObject(QStringLiteral("Event"));
    resFor.setProperty(QStringLiteral("location"), newPlace(QStringLiteral("Place")));

    auto res = newObject(QStringLiteral("EventReservation"));
    res.setProperty(QStringLiteral("reservationFor"), resFor);

    auto ticket = newObject(QStringLiteral("Ticket"));
    ticket.setProperty(QStringLiteral("ticketedSeat"), newObject(QStringLiteral("Seat")));
    res.setProperty(QStringLiteral("reservedTicket"), ticket);

    const auto person = newObject(QStringLiteral("Person"));
    res.setProperty(QStringLiteral("underName"), person);

    return res;
}

QJSValue JsApi::JsonLd::newFoodEstablishmentReservation() const
{
    const auto person = newObject(QStringLiteral("Person"));
    const auto resFor = newPlace(QStringLiteral("FoodEstablishment"));

    auto res = newObject(QStringLiteral("FoodEstablishmentReservation"));
    res.setProperty(QStringLiteral("reservationFor"), resFor);
    res.setProperty(QStringLiteral("underName"), person);

    return res;
}

QJSValue JsApi::JsonLd::newRentalCarReservation() const
{
    const auto pickup = newPlace(QStringLiteral("Place"));
    const auto dropoff = newPlace(QStringLiteral("Place"));
    const auto person = newObject(QStringLiteral("Person"));
    const auto org = newObject(QStringLiteral("Organization"));

    auto resFor = newObject(QStringLiteral("RentalCar"));
    resFor.setProperty(QStringLiteral("rentalCompany"), org);

    auto res = newObject(QStringLiteral("RentalCarReservation"));
    res.setProperty(QStringLiteral("reservationFor"), resFor);
    res.setProperty(QStringLiteral("pickupLocation"), pickup);
    res.setProperty(QStringLiteral("dropoffLocation"), dropoff);
    res.setProperty(QStringLiteral("underName"), person);

    return res;
}

QJSValue JsApi::JsonLd::newBoatReservation() const
{
    const auto dep = newPlace(QStringLiteral("BoatTerminal"));
    const auto arr = newPlace(QStringLiteral("BoatTerminal"));
    const auto person = newObject(QStringLiteral("Person"));
    const auto ticket = newObject(QStringLiteral("Ticket"));

    auto resFor = newObject(QStringLiteral("BoatTrip"));
    resFor.setProperty(QStringLiteral("departureBoatTerminal"), dep);
    resFor.setProperty(QStringLiteral("arrivalBoatTerminal"), arr);

    auto res = newObject(QStringLiteral("BoatReservation"));
    res.setProperty(QStringLiteral("reservationFor"), resFor);
    res.setProperty(QStringLiteral("underName"), person);
    res.setProperty(QStringLiteral("reservedTicket"), ticket);

    return res;
}

QJSValue JsApi::JsonLd::trainToBusReservation(const QJSValue &trainRes) const
{
    return m_engine->toScriptValue(ReservationConverter::trainToBus(QJsonValue::fromVariant(trainRes.toVariant()).toObject()));
}

QJSValue JsApi::JsonLd::busToTrainReservation(const QJSValue &busRes) const
{
    return m_engine->toScriptValue(ReservationConverter::busToTrain(QJsonValue::fromVariant(busRes.toVariant()).toObject()));
}

QDateTime JsApi::JsonLd::toDateTime(const QString &dtStr, const QString &format, const QString &localeName) const
{
    QLocale locale(localeName);
    auto dt = locale.toDateTime(dtStr, format);

    // try harder for the "MMM" month format
    // QLocale expects the exact string in QLocale::shortMonthName(), while we often encounter a three
    // letter month identifier. For en_US that's the same, for Swedish it isn't though for example. So
    // let's try to fix up the month identifiers to the full short name.
    if (!dt.isValid() && format.contains(QLatin1StringView("MMM"))) {
      auto dtStrFixed = dtStr;
      for (int i = 1; i <= 12; ++i) {
        const auto monthName = locale.monthName(i, QLocale::ShortFormat);
        dtStrFixed.replace(monthName.left(3), monthName, Qt::CaseInsensitive);
      }
      dt = locale.toDateTime(dtStrFixed, format);
    }

    // try even harder for "MMM" month format
    // in the de_DE locale we encounter sometimes almost arbitrary abbreviations for month
    // names (eg. Mrz, Mär for März). So try to identify those and replace them with what QLocale
    // expects
    if (!dt.isValid() && format.contains(QLatin1StringView("MMM"))) {
      auto dtStrFixed = dtStr;
      for (int i = 1; i <= 12; ++i) {
        const auto monthName = locale.monthName(i, QLocale::LongFormat);
        const auto beginIdx = dtStr.indexOf(monthName.at(0));
        if (beginIdx < 0) {
          continue;
        }
        auto endIdx = beginIdx;
        for (auto nameIdx = 0;
             endIdx < dtStr.size() && nameIdx < monthName.size(); ++nameIdx) {
          if (dtStr.at(endIdx).toCaseFolded() ==
              monthName.at(nameIdx).toCaseFolded()) {
            ++endIdx;
          }
        }
        if (endIdx - beginIdx >= 3) {
          dtStrFixed.replace(beginIdx, endIdx - beginIdx,
                             locale.monthName(i, QLocale::ShortFormat));
          break;
        }
      }
      dt = locale.toDateTime(dtStrFixed, format);
    }

    if (!dt.isValid()) {
        return dt;
    }

    const bool hasFullYear = format.contains(QLatin1StringView("yyyy"));
    const bool hasYear =
        hasFullYear || format.contains(QLatin1StringView("yy"));
    const bool hasMonth = format.contains(QLatin1Char('M'));
    const bool hasDay = format.contains(QLatin1Char('d'));

    // time only, set a default date
    if (!hasDay && !hasMonth && !hasYear) {
        dt.setDate({1970, 1, 1});
    }

    // if the date does not contain a year number, determine that based on the context date, if set
    else if (!hasYear && m_contextDate.isValid()) {
        dt.setDate({m_contextDate.date().year(), dt.date().month(), dt.date().day()});
        // go one day back to leave a bit of room for documents produced very close to
        // or even during the trip
        if (dt < m_contextDate.addDays(-1)) {
            dt = dt.addYears(1);
        }
    }

    // fix two-digit years ending up in the wrong century
    else if (!hasFullYear && dt.date().year() / 100 == 19) {
        // be careful to only change the date, QDateTime::addYears can change
        // the time as well if e.g. DST has been changed in the corresponding timezone
        dt.setDate(dt.date().addYears(100));
    }

    return dt;
}

QDateTime JsApi::JsonLd::toDateTime(const QString &dtStr, const QJSValue &format, const QJSValue &localeName) const
{
    if (localeName.isString() && format.isString()) {
        return toDateTime(dtStr, format.toString(), localeName.toString());
    }
    if (format.isArray()) {
      const auto count = format.property(QLatin1StringView("length")).toInt();
      for (auto i = 0; i < count; ++i) {
        const auto dt = toDateTime(dtStr, format.property(i), localeName);
        if (dt.isValid()) {
          return dt;
        }
        }
    }
    if (format.isString() && localeName.isArray()) {
      const auto count =
          localeName.property(QLatin1StringView("length")).toInt();
      for (auto i = 0; i < count; ++i) {
        const auto dt = toDateTime(dtStr, format.toString(),
                                   localeName.property(i).toString());
        if (dt.isValid()) {
          return dt;
        }
        }
    }
    return {};
}

QJSValue JsApi::JsonLd::toJson(const QVariant &v) const
{
    if (v.canConvert<QList<QVariant>>()) {
        return m_engine->toScriptValue(
            JsonLdDocument::toJson(v.value<QList<QVariant>>()));
    }

    const auto json = JsonLdDocument::toJsonValue(v);
    return m_engine->toScriptValue(json);
}

QJSValue JsApi::JsonLd::clone(const QJSValue& v) const
{
    return m_engine->toScriptValue(v.toVariant());
}

QJSValue JsApi::JsonLd::toGeoCoordinates(const QString &mapUrl)
{
    QUrl url(mapUrl);
    if (url.host().contains(QLatin1StringView("google"))) {
      QRegularExpression regExp(
          QStringLiteral("[/=](-?\\d+\\.\\d+),(-?\\d+\\.\\d+)"));
      auto match = regExp.match(url.path());
      if (!match.hasMatch()) {
        match = regExp.match(url.query());
      }

      if (match.hasMatch()) {
        auto geo = m_engine->newObject();
        geo.setProperty(QStringLiteral("@type"),
                        QStringLiteral("GeoCoordinates"));
        geo.setProperty(QStringLiteral("latitude"),
                        match.capturedView(1).toDouble());
        geo.setProperty(QStringLiteral("longitude"),
                        match.capturedView(2).toDouble());
        return geo;
      }
    }

    return {};
}

QJSValue JsApi::JsonLd::readQDateTime(const QVariant &obj, const QString &propName) const
{
    const auto mo = QMetaType(obj.userType()).metaObject();
    if (!mo) {
        return {};
    }
    const auto propIdx = mo->indexOfProperty(propName.toUtf8().constData());
    if (propIdx < 0) {
        qWarning() << "Unknown property name:" << mo->className() << propName;
        return {};
    }
    const auto prop = mo->property(propIdx);
    const auto dt = prop.readOnGadget(obj.constData());
    return toJson(dt);
}

QJSValue JsApi::JsonLd::apply(const QJSValue &lhs, const QJSValue &rhs) const
{
    const auto lhsVar = JsonLdDocument::fromJsonSingular(QJsonValue::fromVariant(lhs.toVariant()).toObject());
    const auto rhsVar = JsonLdDocument::fromJsonSingular(QJsonValue::fromVariant(rhs.toVariant()).toObject());
    const auto v = JsonLdDocument::apply(lhsVar, rhsVar);
    return toJson(v);
}

void JsApi::JsonLd::setContextDate(const QDateTime& dt)
{
    m_contextDate = dt;
}

#include "moc_jsonld.cpp"
