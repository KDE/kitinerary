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
#include "logging.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>
#include <QString>
#include <QUrl>
#include <QXmlStreamReader>

using namespace KItinerary;

namespace KItinerary {
class StructuredDataExtractorPrivate {
public:
    /* Try to parse using an actual XML parser. */
    bool parseXml(const QString &text);
    /* Try to find application/ld+json content with basic string search. */
    bool findLdJson(const QString &text);
    /* Try to fix some common HTML4 damage to make @p text consumable for parseXml(). */
    void fixupHtml4Soft(QString &text) const;
    void fixupHtml4Desperate(QString &text) const;
    bool increaseDesperation(QString &text);

    /* Strip leading content before what looks like the first occurance of microdata. */
    bool stripLeadingContent(QString &text) const;
    /* Recursive microdata parsing. */
    QJsonObject parseMicroData(QXmlStreamReader &reader);
    /* Element-dependent Microdata property value. */
    QString valueForItemProperty(QXmlStreamReader &reader) const;
    void parseJson(const QByteArray &data);
    QByteArray fixupJson(const QByteArray &data) const;

    uint64_t m_parserOffset = 0;
    QJsonArray m_data;
    int m_desperationLevel = 0;
};
}

StructuredDataExtractor::StructuredDataExtractor()
    : d(new StructuredDataExtractorPrivate)
{
}

StructuredDataExtractor::StructuredDataExtractor(StructuredDataExtractor&&) = default;
StructuredDataExtractor::~StructuredDataExtractor() = default;

void StructuredDataExtractor::parse(const QString &text)
{
    // assume a more or less well-formed input and see what we find
    if (d->parseXml(text)) {
        return;
    }

    if (d->findLdJson(text)) {
        return;
    }

    // no luck, check if we have any chance at all
    if (!text.contains(QLatin1String("http://schema.org"))) {
        return;
    }

    // now try the expensive desperate stuff
    auto fixedText = text;
    d->m_parserOffset = 0;
    d->m_desperationLevel = 0;

    qCDebug(Log) << "Trying to strip leading garbage";
    while (d->stripLeadingContent(fixedText)) {
        while (!d->parseXml(fixedText)) {
            if (!d->increaseDesperation(fixedText)) {
                return;
            }
        }
    }
}

QJsonArray StructuredDataExtractor::data() const
{
    return d->m_data;
}

bool StructuredDataExtractorPrivate::parseXml(const QString &text)
{
    QXmlStreamReader reader(text);
    while (!reader.atEnd()) {
        if (reader.tokenType() == QXmlStreamReader::StartElement) {
            m_parserOffset = std::max<uint64_t>(0, reader.characterOffset() - 1);

            // JSON-LD
            if (reader.name() == QLatin1String("script") && reader.attributes().value(QLatin1String("type")) == QLatin1String("application/ld+json")) {
                const auto jsonData = reader.readElementText(QXmlStreamReader::IncludeChildElements);
                parseJson(jsonData.toUtf8());
            }

            // Microdata
            const auto itemType = reader.attributes().value(QLatin1String("itemtype")).toString();
            if (itemType.startsWith(QLatin1String("http://schema.org/"))) {
                auto obj = parseMicroData(reader);
                if (obj.isEmpty()) {
                    continue;
                }
                obj.insert(QStringLiteral("@context"), QStringLiteral("http://schema.org"));
                const QUrl typeUrl(itemType);
                obj.insert(QStringLiteral("@type"), typeUrl.fileName());
                m_data.push_back(obj);
                continue;
            }
        }
        reader.readNext();
    }

    if (reader.hasError()) {
        qCDebug(Log) << reader.errorString() << reader.lineNumber() << reader.columnNumber();
    }

    return !m_data.isEmpty();
}

bool StructuredDataExtractorPrivate::findLdJson(const QString &text)
{
    for (int i = 0; i < text.size();) {
        i = text.indexOf(QLatin1String("<script"), i, Qt::CaseInsensitive);
        if (i < 0) {
            break;
        }
        i = text.indexOf(QLatin1String("type=\"application/ld+json\""), i, Qt::CaseInsensitive);
        if (i < 0) {
            break;
        }
        auto begin = text.indexOf(QLatin1Char('>'), i) + 1;
        if (i < 0) {
            break;
        }
        i = text.indexOf(QLatin1String("</script>"), begin, Qt::CaseInsensitive);
        const auto jsonData = text.mid(begin, i - begin);
        parseJson(jsonData.toUtf8());
    }

    return !m_data.isEmpty();
}

void StructuredDataExtractorPrivate::fixupHtml4Soft(QString &text) const
{
    // fix value-less attributes
    text.replace(QRegularExpression(QStringLiteral("(<[^>]+ )itemscope( [^>]*>)")), QStringLiteral("\\1itemscope=\"\"\\2"));
}

void StructuredDataExtractorPrivate::fixupHtml4Desperate(QString &text) const
{
    // close single-element tags
    text.replace(QRegularExpression(QStringLiteral("(<meta[^>]*[^>/])>")), QStringLiteral("\\1/>"));
    text.replace(QRegularExpression(QStringLiteral("(<link[^>]*[^>/])>")), QStringLiteral("\\1/>"));

    // fix unencoded entities in url attributes
    QRegularExpression hrefRE(QStringLiteral("href=\"[^\"]*&[^;\"]*\""));
    for (auto idx = 0; idx < text.size();) {
        const auto match = hrefRE.match(text, idx, QRegularExpression::NormalMatch, QRegularExpression::DontCheckSubjectStringMatchOption);
        if (!match.hasMatch()) {
            break;
        }
        auto fixedHref = text.mid(match.capturedStart() + 6, match.capturedLength() - 7);
        fixedHref.replace(QLatin1Char('&'), QLatin1String("&amp;"));
        text.replace(match.capturedStart() + 6, match.capturedLength() - 7, fixedHref);
        idx = match.capturedEnd();
    }

    // TODO remove legacy entities like &nbsp;
}

bool StructuredDataExtractorPrivate::increaseDesperation(QString& text)
{
    switch (m_desperationLevel++) {
        case 0:
            qCDebug(Log) << "Trying to fix HTML4 content";
            fixupHtml4Soft(text);
            break;
        case 1:
            qCDebug(Log) << "Desperately trying to fix HTML4 content";
            fixupHtml4Desperate(text);
            break;
        default:
            return false;
    }
    return true;
}

bool StructuredDataExtractorPrivate::stripLeadingContent(QString &text) const
{
    auto idx = text.indexOf(QLatin1String("http://schema.org"), m_parserOffset);
    if (idx < 0) {
        return false;
    }

    idx = text.midRef(0, idx).lastIndexOf(QLatin1Char('<'));
    if (idx <= 0) {
        return false;
    }

    text.remove(0, idx);
    return true;
}

QJsonObject StructuredDataExtractorPrivate::parseMicroData(QXmlStreamReader &reader)
{
    QJsonObject obj;
    reader.readNext();
    int depth = 1;

    while (!reader.atEnd()) {
        m_parserOffset = std::max<uint64_t>(0, reader.characterOffset() - 1);
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
            if (depth == 0) {
                return obj;
            }
        }
        reader.readNext();
    }

    if (reader.hasError()) {
        qCDebug(Log) << reader.errorString() << reader.lineNumber() << reader.columnNumber();
    }
    return {};
}

QString StructuredDataExtractorPrivate::valueForItemProperty(QXmlStreamReader &reader) const
{
    // TODO see https://developer.mozilla.org/en-US/docs/Web/HTML/Global_attributes/itemprop#Values
    const auto elemName = reader.name();
    if (elemName == QLatin1String("span")) {
        return reader.readElementText(QXmlStreamReader::IncludeChildElements);
    }

    QString v;
    if (elemName == QLatin1String("meta")) {
        v = reader.attributes().value(QLatin1String("content")).toString();
    } else if (elemName == QLatin1String("time")) {
        v = reader.attributes().value(QLatin1String("datetime")).toString();
    } else if (elemName == QLatin1String("link") || elemName == QLatin1String("a")) {
        if (reader.attributes().hasAttribute(QLatin1String("href"))) {
            v = reader.attributes().value(QLatin1String("href")).toString();
        } else if (reader.attributes().hasAttribute(QLatin1String("content"))) {
            v = reader.attributes().value(QLatin1String("content")).toString();
        }
    } else {
        qCDebug(Log) << "TODO:" << elemName;
    }

    reader.readNext();
    return v;
}

void StructuredDataExtractorPrivate::parseJson(const QByteArray &data)
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
        for (const auto &v : jsonDoc.array()) {
            m_data.push_back(v);
        }
    } else if (jsonDoc.isObject()) {
        m_data.push_back(jsonDoc.object());
    }
}

QByteArray StructuredDataExtractorPrivate::fixupJson(const QByteArray &data) const
{
    auto output(data);

    // Eurowings doesn't put a comma between objects in top-level arrays...
    output.replace("}{", "},{");

    return output;
}
