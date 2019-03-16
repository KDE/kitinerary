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

#include "structureddataextractor_p.h"
#include "htmldocument.h"
#include "logging.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QString>
#include <QUrl>

using namespace KItinerary;

static QByteArray fixupJson(const QByteArray &data)
{
    auto output(data);

    // Eurowings doesn't put a comma between objects in top-level arrays...
    output.replace("}{", "},{");

    return output;
}

static void parseJson(const QByteArray &data, QJsonArray &result)
{
    QJsonParseError error;
    auto jsonDoc = QJsonDocument::fromJson(data, &error);
    if (jsonDoc.isNull()) {
        if (error.error != QJsonParseError::NoError) {
            // try to fix up common JSON encoding errors
            jsonDoc = QJsonDocument::fromJson(fixupJson(data));
        }
        if (jsonDoc.isNull()) {
            qCDebug(Log).noquote() << data;
            qCDebug(Log) << error.errorString() << "at offset" << error.offset;
            return;
        }
    }
    if (jsonDoc.isArray()) {
        const auto jsonArray = jsonDoc.array();
        std::copy(jsonArray.begin(), jsonArray.end(), std::back_inserter(result));
    } else if (jsonDoc.isObject()) {
        result.push_back(jsonDoc.object());
    }
}

static QString valueForItemProperty(const HtmlElement &elem)
{
    // TODO see https://developer.mozilla.org/en-US/docs/Web/HTML/Global_attributes/itemprop#Values
    const auto elemName = elem.name();
    QString v;
    if (elemName == QLatin1String("meta")) {
        v = elem.attribute(QStringLiteral("content"));
    } else if (elemName == QLatin1String("time")) {
        v = elem.attribute(QStringLiteral("datetime"));
    } else if (elemName == QLatin1String("link") || elemName == QLatin1String("a")) {
        if (elem.hasAttribute(QStringLiteral("href"))) {
            v = elem.attribute(QStringLiteral("href"));
        } else if (elem.hasAttribute(QStringLiteral("content"))) {
            v = elem.attribute(QStringLiteral("content"));
        } else {
            v = elem.recursiveContent();
        }
    } else {
        v = elem.recursiveContent();
    }

    return v;
}

static void parseMicroData(const HtmlElement &elem, QJsonObject &obj)
{
    auto child = elem.firstChild();
    while (!child.isNull()) {
        const auto prop = child.attribute(QStringLiteral("itemprop"));
        const auto type = child.attribute(QStringLiteral("itemtype"));
        if (type.startsWith(QLatin1String("http://schema.org/"))) {
            QJsonObject subObj;
            parseMicroData(child, subObj);
            const QUrl typeUrl(type);
            subObj.insert(QStringLiteral("@type"), typeUrl.fileName());
            obj.insert(prop, subObj);
        } else if (!prop.isEmpty()) {
            obj.insert(prop, valueForItemProperty(child));
        } else  {
            // skip intermediate nodes without Microdata annotations
            parseMicroData(child, obj);
        }
        child = child.nextSibling();
    }
}

static void extractRecursive(const HtmlElement &elem, QJsonArray &result)
{
    // JSON-LD
    if (elem.name() == QLatin1String("script") && elem.attribute(QStringLiteral("type")) == QLatin1String("application/ld+json")) {
        parseJson(elem.content().toUtf8(), result);
        return;
    }

    // Microdata
    const auto itemType = elem.attribute(QStringLiteral("itemtype"));
    if (itemType.startsWith(QLatin1String("http://schema.org/"))) {
        QJsonObject obj;
        parseMicroData(elem, obj);
        if (obj.isEmpty()) {
            return;
        }

        const QUrl typeUrl(itemType);
        obj.insert(QStringLiteral("@type"), typeUrl.fileName());

        const auto itemProp = elem.attribute(QStringLiteral("itemprop"));
        if (!itemProp.isEmpty() && !result.isEmpty()) {
            // this is likely a child of our preceding sibling, but broken XML put it here
            auto parent = result.last().toObject();
            parent.insert(itemProp, obj);
            result[result.size() - 1] = parent;
        } else {
            obj.insert(QStringLiteral("@context"), QStringLiteral("http://schema.org"));
            result.push_back(obj);
        }
        return;
    }

    // recurse otherwise
    auto child = elem.firstChild();
    while (!child.isNull()) {
        extractRecursive(child, result);
        child = child.nextSibling();
    }
}

QJsonArray StructuredDataExtractor::extract(HtmlDocument *doc)
{
    Q_ASSERT(doc);

    QJsonArray result;
    if (doc->root().isNull()) {
        return result;
    }
    extractRecursive(doc->root(), result);
    return result;
}
