/*
    Copyright (C) 2018 Volker Krause <vkrause@kde.org>

    This program is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This program is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "jsonld.h"

#include <KItinerary/JsonLdDocument>

#include <QDateTime>
#include <QDebug>
#include <QJSEngine>
#include <QJsonArray>
#include <QLocale>
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

QDateTime JsApi::JsonLd::toDateTime(const QString &dtStr, const QString &format, const QString &localeName) const
{
    QLocale locale(localeName);
    auto dt = locale.toDateTime(dtStr, format);

    // try harder for the "MMM" month format
    // QLocale expects the exact string in QLocale::shortMonthName(), while we often encounter a three
    // letter month identifier. For en_US that's the same, for Swedish it isn't though for example. So
    // let's try to fix up the month identifiers to the full short name.
    if (!dt.isValid() && format.contains(QLatin1String("MMM"))) {
        auto dtStrFixed = dtStr;
        for (int i = 1; i <= 12; ++i) {
            const auto monthName = locale.monthName(i, QLocale::ShortFormat);
            dtStrFixed = dtStrFixed.replace(monthName.left(3), monthName, Qt::CaseInsensitive);
        }
        dt = locale.toDateTime(dtStrFixed, format);
    }

    // try even harder for "MMM" month format
    // in the de_DE locale we encounter sometimes almost arbitrary abbreviations for month
    // names (eg. Mrz, Mär for März). So try to identify those and replace them with what QLocale
    // expects
    if (!dt.isValid() && format.contains(QLatin1String("MMM"))) {
        auto dtStrFixed = dtStr;
        for (int i = 1; i <= 12; ++i) {
            const auto monthName = locale.monthName(i, QLocale::LongFormat);
            const auto beginIdx = dtStr.indexOf(monthName.at(0));
            if (beginIdx < 0) {
                continue;
            }
            auto endIdx = beginIdx;
            for (auto nameIdx = 0; endIdx < dtStr.size() && nameIdx < monthName.size(); ++nameIdx) {
                if (dtStr.at(endIdx).toCaseFolded() == monthName.at(nameIdx).toCaseFolded()) {
                    ++endIdx;
                }
            }
            if (endIdx - beginIdx >= 3) {
                dtStrFixed = dtStrFixed.replace(beginIdx, endIdx - beginIdx, locale.monthName(i, QLocale::ShortFormat));
                break;
            }
        }
        dt = locale.toDateTime(dtStrFixed, format);
    }

    if (!dt.isValid()) {
        return dt;
    }

    const bool hasFullYear = format.contains(QLatin1String("yyyy"));
    const bool hasYear = hasFullYear || format.contains(QLatin1String("yy"));
    const bool hasMonth = format.contains(QLatin1String("M"));
    const bool hasDay = format.contains(QLatin1String("d"));

    // time only, set a default date
    if (!hasDay && !hasMonth && !hasYear) {
        dt.setDate({1970, 1, 1});
    }

    // if the date does not contain a year number, determine that based on the context date, if set
    else if (!hasYear && m_contextDate.isValid()) {
        dt.setDate({m_contextDate.date().year(), dt.date().month(), dt.date().day()});
        if (dt < m_contextDate) {
            dt = dt.addYears(1);
        }
    }

    // fix two-digit years ending up in the wrong century
    else if (!hasFullYear && dt.date().year() / 100 == 19) {
        dt = dt.addYears(100);
    }

    return dt;
}

QJSValue JsApi::JsonLd::toJson(const QVariant &v) const
{
    if (v.canConvert<QVector<QVariant>>()) {
        return m_engine->toScriptValue(JsonLdDocument::toJson(v.value<QVector<QVariant>>()));
    }

    const auto json = JsonLdDocument::toJson({v});
    if (json.isEmpty()) {
        return {};
    }
    return m_engine->toScriptValue(json.at(0));
}

QJSValue JsApi::JsonLd::clone(const QJSValue& v) const
{
    return m_engine->toScriptValue(v.toVariant());
}

QJSValue JsApi::JsonLd::toGeoCoordinates(const QString &mapUrl)
{
    QUrl url(mapUrl);
    if (url.host().contains(QLatin1String("google"))) {
        QRegularExpression regExp(QStringLiteral("[/=](-?\\d+\\.\\d+),(-?\\d+\\.\\d+)"));
        auto match = regExp.match(url.path());
        if (!match.hasMatch()) {
            match = regExp.match(url.query());
        }

        if (match.hasMatch()) {
            auto geo = m_engine->newObject();
            geo.setProperty(QStringLiteral("@type"), QStringLiteral("GeoCoordinates"));
            geo.setProperty(QStringLiteral("latitude"), match.capturedRef(1).toDouble());
            geo.setProperty(QStringLiteral("longitude"), match.capturedRef(2).toDouble());
            return geo;
        }
    }

    return {};
}

void JsApi::JsonLd::setContextDate(const QDateTime& dt)
{
    m_contextDate = dt;
}

#include "moc_jsonld.cpp"
