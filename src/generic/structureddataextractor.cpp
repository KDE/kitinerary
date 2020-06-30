/*
   SPDX-FileCopyrightText: 2017 Volker Krause <vkrause@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
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

static bool isJsonLdTag(const HtmlElement &elem)
{
    return elem.name() == QLatin1String("script") && elem.attribute(QStringLiteral("type")) == QLatin1String("application/ld+json");
}

static QByteArray fixupJson(const QByteArray &data)
{
    auto output(data);

    // Eurowings doesn't put a comma between objects in top-level arrays...
    output.replace("}{", "},{");

    // Volotea doesn't put square brackets in top level arrays...
    if (output.front() != '[' && output.back() != ']') {
        output.prepend("[");
        output.append("]");
    }

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
    } else if (elemName == QLatin1String("link") || elemName == QLatin1Char('a') || elemName == QLatin1String("img")) {
        if (elem.hasAttribute(QStringLiteral("href"))) {
            v = elem.attribute(QStringLiteral("href"));
        } else if (elem.hasAttribute(QStringLiteral("content"))) {
            v = elem.attribute(QStringLiteral("content"));
        } else if (elem.hasAttribute(QStringLiteral("src"))) {
            v = elem.attribute(QStringLiteral("src"));
        } else {
            v = elem.recursiveContent();
        }
    } else {
        v = elem.recursiveContent();
    }

    return v;
}

static void parseMicroData(const HtmlElement &elem, QJsonObject &obj, QJsonArray &result)
{
    auto child = elem.firstChild();
    while (!child.isNull()) {
        const auto prop = child.attribute(QStringLiteral("itemprop"));
        const auto type = child.attribute(QStringLiteral("itemtype"));
        if (type.startsWith(QLatin1String("http://schema.org/"))) {
            QJsonObject subObj;
            parseMicroData(child, subObj, result);
            const QUrl typeUrl(type);
            subObj.insert(QStringLiteral("@type"), typeUrl.fileName());
            if (prop.isEmpty()) {
                result.push_back(subObj); // stand-alone object that just happens to be nested
            } else {
                obj.insert(prop, subObj);
            }
        } else if (!prop.isEmpty()) {
            obj.insert(prop, valueForItemProperty(child));
        // Maybe there is more JSON-LD inside this microdata tree
        } else if (isJsonLdTag(child)) {
            parseJson(child.content().toUtf8(), result);
        } else {
            // skip intermediate nodes without Microdata annotations
            parseMicroData(child, obj, result);
        }
        child = child.nextSibling();
    }
}

static void extractRecursive(const HtmlElement &elem, QJsonArray &result)
{
    // JSON-LD
    if (isJsonLdTag(elem)) {
        parseJson(elem.content().toUtf8(), result);
        return;
    }

    // Microdata
    const auto itemType = elem.attribute(QStringLiteral("itemtype"));
    if (itemType.startsWith(QLatin1String("http://schema.org/"))) {
        QJsonObject obj;
        parseMicroData(elem, obj, result);
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
