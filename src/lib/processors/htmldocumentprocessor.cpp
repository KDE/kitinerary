/*
   SPDX-FileCopyrightText: 2017-2021 Volker Krause <vkrause@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "htmldocumentprocessor.h"
#include "logging.h"

#include <KItinerary/ExtractorDocumentNodeFactory>
#include <KItinerary/ExtractorEngine>
#include <KItinerary/ExtractorResult>
#include <KItinerary/HtmlDocument>

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJSEngine>
#include <QJSValue>
#include <QString>
#include <QUrl>

using namespace KItinerary;

Q_DECLARE_METATYPE(KItinerary::Internal::OwnedPtr<KItinerary::HtmlDocument>)

static bool contentStartsWith(const QByteArray &data, char s)
{
    for (const auto c : data) {
        if (std::isspace(c)) {
            continue;
        }
        return c == s;
    }
    return false;
}

bool HtmlDocumentProcessor::canHandleData(const QByteArray &encodedData, QStringView fileName) const
{
    return contentStartsWith(encodedData, '<')
        || fileName.endsWith(QLatin1String(".html"), Qt::CaseInsensitive)
        || fileName.endsWith(QLatin1String(".htm"), Qt::CaseInsensitive);
}

ExtractorDocumentNode HtmlDocumentProcessor::createNodeFromData(const QByteArray &encodedData) const
{
    auto html = HtmlDocument::fromData(encodedData);
    if (!html || html->root().firstChild().isNull()) {
        return {};
    }

    ExtractorDocumentNode node;
    node.setContent<Internal::OwnedPtr<HtmlDocument>>(html);
    return node;
}

void HtmlDocumentProcessor::expandNode(ExtractorDocumentNode &node, const ExtractorEngine *engine) const
{
    // plain text fallback node
    const auto html = node.content<HtmlDocument*>();
    auto fallback = engine->documentNodeFactory()->createNode(html->root().recursiveContent(), u"text/plain");
    node.appendChild(fallback);
}

static bool isJsonLdTag(const HtmlElement &elem)
{
    return elem.name() == QLatin1String("script") && elem.attribute(QStringLiteral("type")) == QLatin1String("application/ld+json");
}

static QByteArray fixupJson(const QByteArray &data)
{
    if (data.isEmpty()) {
        return {};
    }
    auto output(data);

    // Eurowings doesn't put a comma between objects in top-level arrays...
    output.replace("}{", "},{");

    // Volotea doesn't put square brackets in top level arrays...
    if (output.front() != '[' && output.back() != ']') {
        output.prepend("[");
        output.append("]");
    }

    // Eventbrite adds commas where there shouldn't be one...
    for (int idx = output.indexOf("\",\n"); idx > 0 && idx + 3 < output.size(); idx = output.indexOf("\",\n", idx)) {
        const auto comma = idx + 1;
        idx += 3;
        while (idx < output.size() && std::isspace(output[idx])) {
            ++idx;
        }
        if (idx < output.size() && output[idx] == '}') {
            output[comma] = ' ';
        }
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

static void insertProperties(QJsonObject &obj, const QString &prop, const QJsonValue &val)
{
    // multiple properties can be specified at once, as a space-separated list
    const auto props = prop.split(QLatin1Char(' '), Qt::SkipEmptyParts);
    for (const auto &p : props) {
        obj.insert(p, val);
    }
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
                insertProperties(obj, prop, subObj);
            }
        } else if (!prop.isEmpty()) {
            insertProperties(obj, prop, valueForItemProperty(child));
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

void HtmlDocumentProcessor::preExtract(ExtractorDocumentNode &node, [[maybe_unused]] const ExtractorEngine *engine) const
{
    auto doc = node.content<HtmlDocument*>();
    Q_ASSERT(doc);

    if (!doc->root().isNull()) {
        QJsonArray result;
        extractRecursive(doc->root(), result);
        node.addResult(result);
    }
}

QJSValue HtmlDocumentProcessor::contentToScriptValue(const ExtractorDocumentNode &node, QJSEngine *engine) const
{
    return engine->toScriptValue(node.content<HtmlDocument*>());
}

void HtmlDocumentProcessor::destroyNode(ExtractorDocumentNode &node) const
{
    destroyIfOwned<HtmlDocument>(node);
}