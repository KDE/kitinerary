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
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "jsonld.h"

#include <KItinerary/JsonLdDocument>

#include <QDateTime>
#include <QJSEngine>
#include <QJsonArray>
#include <QLocale>

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

// two digit year numbers end up in the last century, fix that
static QDateTime fixupDate(const QDateTime &dt)
{
    if (dt.date().year() / 100 == 19) {
        return dt.addYears(100);
    }
    return dt;
}

QDateTime JsApi::JsonLd::toDateTime(const QString &dtStr, const QString &format, const QString &localeName) const
{
    QLocale locale(localeName);
    const auto dt = locale.toDateTime(dtStr, format);
    if (dt.isValid()) {
        return fixupDate(dt);
    }

    // try harder for the "MMM" month format
    // QLocale expects the exact string in QLocale::shortMonthName(), while we often encounter a three
    // letter month identifier. For en_US that's the same, for Swedish it isn't though for example. So
    // let's try to fix up the month identifiers to the full short name.
    if (format.contains(QLatin1String("MMM"))) {
        auto dtStrFixed = dtStr;
        for (int i = 0; i < 12; ++i) {
            const auto monthName = locale.monthName(i, QLocale::ShortFormat);
            dtStrFixed = dtStrFixed.replace(monthName.left(3), monthName);
        }
        return fixupDate(locale.toDateTime(dtStrFixed, format));
    }
    return dt;
}

QJSValue JsApi::JsonLd::toJson(const QVariant &v) const
{
    const auto json = JsonLdDocument::toJson({v});
    if (json.isEmpty()) {
        return {};
    }
    return m_engine->toScriptValue(json.at(0));
}

#include "moc_jsonld.cpp"
