/*
   SPDX-FileCopyrightText: 2017-2021 Volker Krause <vkrause@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "htmldocumentprocessor.h"

#include "genericpriceextractorhelper_p.h"
#include "logging.h"
#include "stringutil.h"
#include "json/jsonld.h"

#include <KItinerary/ExtractorDocumentNodeFactory>
#include <KItinerary/ExtractorEngine>
#include <KItinerary/ExtractorResult>
#include <KItinerary/HtmlDocument>
#include <KItinerary/JsonLdDocument>

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJSEngine>
#include <QJSValue>
#include <QString>
#include <QUrl>

#include <cmath>

using namespace KItinerary;

Q_DECLARE_METATYPE(KItinerary::Internal::OwnedPtr<KItinerary::HtmlDocument>)

bool HtmlDocumentProcessor::canHandleData(const QByteArray &encodedData, QStringView fileName) const
{
  return StringUtil::startsWithIgnoreSpace(encodedData, "<") ||
         fileName.endsWith(QLatin1StringView(".html"), Qt::CaseInsensitive) ||
         fileName.endsWith(QLatin1StringView(".htm"), Qt::CaseInsensitive);
}

static ExtractorDocumentNode nodeFromHtml(HtmlDocument *html)
{
    if (!html || html->root().firstChild().isNull()) {
        return {};
    }

    ExtractorDocumentNode node;
    node.setContent<Internal::OwnedPtr<HtmlDocument>>(html);
    return node;
}

ExtractorDocumentNode HtmlDocumentProcessor::createNodeFromData(const QByteArray &encodedData) const
{
    return nodeFromHtml(HtmlDocument::fromData(encodedData));
}

ExtractorDocumentNode HtmlDocumentProcessor::createNodeFromContent(const QVariant &decodedData) const
{
    if (decodedData.userType() == QMetaType::QString) {
        return nodeFromHtml(HtmlDocument::fromString(decodedData.toString()));
    }
    return ExtractorDocumentProcessor::createNodeFromContent(decodedData);
}

void HtmlDocumentProcessor::expandNode(ExtractorDocumentNode &node, const ExtractorEngine *engine) const
{
    const auto html = node.content<HtmlDocument*>();

    // inline images
    expandElementRecursive(node, html->root(), engine);

    // plain text fallback node
    auto fallback = engine->documentNodeFactory()->createNode(html->root().recursiveContent(), u"text/plain");
    node.appendChild(fallback);
}

static bool isJsonLdTag(const HtmlElement &elem)
{
  return elem.name() == QLatin1StringView("script") &&
         elem.attribute(QStringLiteral("type")) ==
             QLatin1StringView("application/ld+json");
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
    for (qsizetype idx = output.indexOf("\",\n"); idx > 0 && idx + 3 < output.size(); idx = output.indexOf("\",\n", idx)) {
        const auto comma = idx + 1;
        idx += 3;
        while (idx < output.size() && std::isspace(static_cast<unsigned char>(output[idx]))) {
            ++idx;
        }
        if (idx < output.size() && output[idx] == '}') {
            output[comma] = ' ';
        }
    }

    // Airbnb applies XML entity encoding...
    output.replace("&quot;", "\"");

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
    if (elemName == QLatin1StringView("meta")) {
      v = elem.attribute(QStringLiteral("content"));
    } else if (elemName == QLatin1StringView("time")) {
      v = elem.attribute(QStringLiteral("datetime"));
    } else if (elemName == QLatin1StringView("link") ||
               elemName == QLatin1Char('a') ||
               elemName == QLatin1StringView("img")) {
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
        auto valRef = obj[p];
        if (valRef.isUndefined() || valRef.isNull()) {
            obj.insert(p, val);
        // convert multiple repeated properties into an array
        } else if (valRef.isArray()) {
            auto array = valRef.toArray();
            array.push_back(val);
            valRef = array;
        } else {
            QJsonArray array({valRef, val});
            valRef = array;
        }
    }
}

static void parseMicroData(const HtmlElement &elem, QJsonObject &obj, QJsonArray &result)
{
    auto child = elem.firstChild();
    while (!child.isNull()) {
        const auto prop = child.attribute(QStringLiteral("itemprop"));
        const auto type = child.attribute(QStringLiteral("itemtype"));
        if (JsonLd::isSchemaOrgNamespace(type)) {
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
    if (JsonLd::isSchemaOrgNamespace(itemType)) {
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

void HtmlDocumentProcessor::postExtract(ExtractorDocumentNode &node, [[maybe_unused]] const ExtractorEngine *engine) const
{
    if (node.childNodes().empty() || node.result().isEmpty()) {
        return;
    }

    const QString text = node.childNodes().back().content<QString>();
    GenericPriceExtractorHelper::postExtract(text, node);
}

QJSValue HtmlDocumentProcessor::contentToScriptValue(const ExtractorDocumentNode &node, QJSEngine *engine) const
{
    return engine->toScriptValue(node.content<HtmlDocument*>());
}

void HtmlDocumentProcessor::destroyNode(ExtractorDocumentNode &node) const
{
    destroyIfOwned<HtmlDocument>(node);
}

void HtmlDocumentProcessor::expandElementRecursive(ExtractorDocumentNode &node, const HtmlElement &elem, const ExtractorEngine *engine) const
{
  if (elem.name() == QLatin1StringView("img")) {
    const auto src = elem.attribute(QLatin1StringView("src"));
    if (src.startsWith(QLatin1StringView("data:"))) {
      expandDataUrl(node, src, engine);
    }
  }

    auto child = elem.firstChild();
    while (!child.isNull()) {
        expandElementRecursive(node, child, engine);
        child = child.nextSibling();
    }
}

void HtmlDocumentProcessor::expandDataUrl(ExtractorDocumentNode &node, QStringView data, const ExtractorEngine *engine) const
{
    const auto idx = data.indexOf(QLatin1Char(','));
    if (idx < 0) {
        return;
    }
    const auto header = data.mid(5, idx - 5);
    const auto headerItems = header.split(QLatin1Char(';'));
    if (headerItems.isEmpty()) {
        return;
    }

    if (headerItems.front() != QLatin1StringView("image/png")) {
      return;
    }

    auto imgData = data.mid(idx).toUtf8();
    if (headerItems.back() == QLatin1StringView("base64")) {
      imgData = QByteArray::fromBase64(imgData.trimmed());
    }

    auto child = engine->documentNodeFactory()->createNode(imgData, {}, headerItems.front());
    node.appendChild(child);
}
