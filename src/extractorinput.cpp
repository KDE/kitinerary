/*
    Copyright (C) 2019 Volker Krause <vkrause@kde.org>

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

#include "extractorinput.h"

#include <QMetaEnum>
#include <QString>

#include <cstring>

using namespace KItinerary;

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

static bool contentStartsWith(const QByteArray &data, const char *str)
{
    auto it = data.begin();
    while (it != data.end() && std::isspace(*it)) {
        ++it;
    }

    const auto len = std::strlen(str);
    if ((int)len >= std::distance(it, data.end())) {
        return false;
    }
    return std::strncmp(it, str, len) == 0;
}

static bool contentMightBeEmail(const QByteArray &data)
{
    // raw email
    for (const auto c : data) {
        if (std::isalpha(c) || c == '-') {
            continue;
        }
        if (c == ':') {
            return true;
        } else {
            break;
        }
    }

    // mbox format
    return data.startsWith("From ");
}


ExtractorInput::Type ExtractorInput::typeFromContent(const QByteArray &content)
{
    if (content.size() < 4) {
        return ExtractorInput::Unknown;
    }

    if (std::strncmp(content.constData(), "PK\x03\x04", 4) == 0) {
        return ExtractorInput::PkPass;
    }
    if (std::strncmp(content.constData(), "%PDF", 4) == 0) {
        return ExtractorInput::Pdf;
    }
    if (contentStartsWith(content, '<')) {
        return ExtractorInput::Html;
    }
    if (contentStartsWith(content, "BEGIN:VCALENDAR")) {
        return ExtractorInput::ICal;
    }
    if (contentMightBeEmail(content)) {
        return ExtractorInput::Email;
    }
    if (contentStartsWith(content, '{') || contentStartsWith(content, '[')) {
        return ExtractorInput::JsonLd;
    }

    return {};
}

ExtractorInput::Type ExtractorInput::typeFromMimeType(const QString &mimeType)
{
    if (mimeType == QLatin1String("application/vnd.apple.pkpass")) {
        return ExtractorInput::PkPass;
    }
    if (mimeType == QLatin1String("text/calendar")) {
        return ExtractorInput::ICal;
    }
    if (mimeType == QLatin1String("application/pdf")) {
        return ExtractorInput::Pdf;
    }
    if (mimeType == QLatin1String("text/html")) {
        return ExtractorInput::Html;
    }
    if (mimeType == QLatin1String("application/json") || mimeType == QLatin1String("application/ld+json")) {
        return ExtractorInput::JsonLd;
    }
    if (mimeType == QLatin1String("message/rfc822")) {
        return ExtractorInput::Email;
    }
    if (mimeType == QLatin1String("text/plain")) {
        return ExtractorInput::Text;
    }
    return {};
}

ExtractorInput::Type ExtractorInput::typeFromFileName(const QString &fileName)
{
    if (fileName.endsWith(QLatin1String(".pkpass"), Qt::CaseInsensitive)) {
        return ExtractorInput::PkPass;
    }
    if (fileName.endsWith(QLatin1String(".ics"), Qt::CaseInsensitive) || fileName.endsWith(QLatin1String(".ical"), Qt::CaseInsensitive)) {
        return ExtractorInput::ICal;
    }
    if (fileName.endsWith(QLatin1String(".pdf"), Qt::CaseInsensitive)) {
        return ExtractorInput::Pdf;
    }
    if (fileName.endsWith(QLatin1String(".html"), Qt::CaseInsensitive) || fileName.endsWith(QLatin1String(".htm"), Qt::CaseInsensitive)) {
        return ExtractorInput::Html;
    }
    if (fileName.endsWith(QLatin1String(".json"), Qt::CaseInsensitive) || fileName.endsWith(QLatin1String(".jsonld"), Qt::CaseInsensitive)) {
        return ExtractorInput::JsonLd;
    }
    if (fileName.endsWith(QLatin1String(".eml"), Qt::CaseInsensitive) || fileName.endsWith(QLatin1String(".mbox"), Qt::CaseInsensitive)) {
        return ExtractorInput::Email;
    }
    if (fileName.endsWith(QLatin1String(".txt"), Qt::CaseInsensitive)) {
        return ExtractorInput::Text;
    }
    return {};
}

QString ExtractorInput::typeToString(ExtractorInput::Type type)
{
    if (type == Unknown) {
        return {};
    }

    const auto me = QMetaEnum::fromType<ExtractorInput::Type>();
    Q_ASSERT(me.isValid());
    return QString::fromUtf8(me.valueToKey(type));
}

ExtractorInput::Type ExtractorInput::typeFromName(const QString &name)
{
    const auto me = QMetaEnum::fromType<ExtractorInput::Type>();
    Q_ASSERT(me.isValid());

    bool ok = false;
    const auto value = me.keyToValue(name.toUtf8().constData(), &ok);
    if (ok && value != Unknown) {
        return static_cast<ExtractorInput::Type>(value);
    }

    for (auto i = 0; i < me.keyCount(); ++i) {
        if (qstricmp(name.toUtf8().constData(), me.key(i)) == 0) {
            return static_cast<ExtractorInput::Type>(me.value(i));
        }
    }

    return {};
}

#include "moc_extractorinput.cpp"
