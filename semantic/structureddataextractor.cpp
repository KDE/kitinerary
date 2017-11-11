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

#include "structureddataextractor.h"
#include "semantic_debug.h"

#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>
#include <QString>
#include <QUrl>
#include <QXmlStreamReader>

void StructuredDataExtractor::parse(const QString &text)
{
    parseXml(text);
    if (m_data.isEmpty()) {
        findLdJson(text);
        if (m_data.isEmpty()) {
            parseXml(fixupHtml4(text));
        }
    }
}

void StructuredDataExtractor::parseXml(const QString &text)
{
    QXmlStreamReader reader(text);
    while (!reader.atEnd()) {
        if (reader.tokenType() == QXmlStreamReader::StartElement) {
            // JSON-LD
            if (reader.name() == QLatin1String("script") && reader.attributes().value(QLatin1String("type")) == QLatin1String("application/ld+json")) {
                const auto jsonData = reader.readElementText(QXmlStreamReader::IncludeChildElements);
                const auto jsonDoc = QJsonDocument::fromJson(jsonData.toUtf8());
                if (jsonDoc.isNull())
                    continue;
                if (jsonDoc.isArray())
                    m_data.append(jsonDoc.array());
                else if (jsonDoc.isObject())
                    m_data.push_back(jsonDoc.object());
            }

            // Microdata
            const auto itemType = reader.attributes().value(QLatin1String("itemtype")).toString();
            if (itemType.startsWith(QLatin1String("http://schema.org/"))) {
                auto obj = parseMicroData(reader);
                if (obj.isEmpty())
                    continue;
                obj.insert(QStringLiteral("@context"), QStringLiteral("http://schema.org"));
                const QUrl typeUrl(itemType);
                obj.insert(QStringLiteral("@type"), typeUrl.fileName());
                m_data.push_back(obj);
                continue;
            }

        }
        reader.readNext();
    }

    if (reader.hasError())
        qCDebug(SEMANTIC_LOG) << reader.errorString() << reader.lineNumber() << reader.columnNumber();
}

void StructuredDataExtractor::findLdJson(const QString &text)
{
    for (int i = 0; i < text.size();) {
        i = text.indexOf(QLatin1String("<script"), i, Qt::CaseInsensitive);
        if (i < 0)
            break;
        i = text.indexOf(QLatin1String("type=\"application/ld+json\""), i, Qt::CaseInsensitive);
        if (i < 0)
            break;
        auto begin = text.indexOf(QLatin1Char('>'), i) + 1;
        if (i < 0)
            break;
        i = text.indexOf(QLatin1String("</script>"), begin, Qt::CaseInsensitive);
        const auto jsonData = text.mid(begin, i - begin);
        auto jsonDoc = QJsonDocument::fromJson(jsonData.toUtf8());
        if (jsonDoc.isNull())
            continue;
        if (jsonDoc.isArray())
            m_data.append(jsonDoc.array());
        else if (jsonDoc.isObject())
            m_data.push_back(jsonDoc.object());
    }
}

QString StructuredDataExtractor::fixupHtml4(const QString &text) const
{
    auto output(text);

    // close single-element tags
    output.replace(QRegularExpression(QStringLiteral("(<meta[^>]*[^>/])>")), QStringLiteral("\\1/>"));

    // fix value-less attributes
    output.replace(QRegularExpression(QStringLiteral("(<[^>]+ )itemscope( [^>]*>)")), QStringLiteral("\\1itemscope=\"\"\\2"));

    // TODO remove legacy entities like &nbsp;
    return output;
}

QJsonObject StructuredDataExtractor::parseMicroData(QXmlStreamReader &reader) const
{
    QJsonObject obj;
    reader.readNext();
    int depth = 1;

    while (!reader.atEnd()) {
        if (reader.tokenType() == QXmlStreamReader::StartElement) {
            ++depth;
            const auto prop = reader.attributes().value(QLatin1String("itemprop")).toString();
            const auto type = reader.attributes().value(QLatin1String("itemtype")).toString();
            if (type.startsWith(QLatin1String("http://schema.org/"))) {
                auto subObj = parseMicroData(reader);
                const QUrl typeUrl(type);
                subObj.insert(QStringLiteral("@type"), typeUrl.fileName());

                obj.insert(prop, subObj);
                continue;
            }
            if (!prop.isEmpty()) {
                obj.insert(prop, valueForItemProperty(reader));
                continue;
            }

        } else if (reader.tokenType() == QXmlStreamReader::EndElement) {
            --depth;
            if (depth == 0)
                return obj;
        }
        reader.readNext();
    }

    if (reader.hasError())
        qCDebug(SEMANTIC_LOG) << reader.errorString() << reader.lineNumber() << reader.columnNumber();
    return {};
}

QString StructuredDataExtractor::valueForItemProperty(QXmlStreamReader &reader) const
{
    // TODO see https://developer.mozilla.org/en-US/docs/Web/HTML/Global_attributes/itemprop#Values
    const auto elemName = reader.name();
    if (elemName == QLatin1String("span"))
        return reader.readElementText(QXmlStreamReader::IncludeChildElements);

    QString v;
    if (elemName == QLatin1String("meta"))
        v = reader.attributes().value(QLatin1String("content")).toString();
    else if (elemName == QLatin1String("time"))
        v = reader.attributes().value(QLatin1String("datetime")).toString();
    else if (elemName == QLatin1String("link") || elemName == QLatin1String("a"))
        v = reader.attributes().value(QLatin1String("href")).toString();
    else
        qCDebug(SEMANTIC_LOG) << "TODO:" << elemName;

    reader.readNext();
    return v;
}
