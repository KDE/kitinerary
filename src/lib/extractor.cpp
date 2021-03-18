/*
   SPDX-FileCopyrightText: 2017-2021 Volker Krause <vkrause@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "extractor.h"
#include "extractorfilter.h"
#include "logging.h"

#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonObject>

#include <memory>

using namespace KItinerary;

namespace KItinerary {
class ExtractorPrivate : public QSharedData
{
public:
    QString m_mimeType;
    QString m_fileName;
    QString m_scriptName;
    QString m_scriptFunction;
    std::vector<ExtractorFilter> m_filters;
    int m_index = -1;
};
}

Extractor::Extractor()
    : d(new ExtractorPrivate)
{
}
Extractor& Extractor::operator=(const Extractor&) = default;
Extractor::Extractor(Extractor&&) noexcept = default;
Extractor::~Extractor() = default;
Extractor::Extractor(const Extractor&) = default;
Extractor& Extractor::operator=(Extractor&&) = default;

bool Extractor::load(const QJsonObject &obj, const QString &fileName, int index)
{
    d->m_fileName = fileName;
    d->m_index = index;

    d->m_mimeType = obj.value(QLatin1String("mimeType")).toString();
    if (d->m_mimeType.isEmpty()) { // ### legacy support
        setType(ExtractorInput::typeFromName(obj.value(QLatin1String("type")).toString()));
    }
    if (d->m_mimeType.isEmpty()) {
        qCDebug(Log) << "extractor MIME type not specified" << fileName;;
    }

    const auto filterArray = obj.value(QLatin1String("filter")).toArray();
    for (const auto &filterValue : filterArray) {
        ExtractorFilter f;
        if (!f.load(filterValue.toObject())) {
            return false;
        }
        d->m_filters.push_back(std::move(f));
    }

    const auto scriptName = obj.value(QLatin1String("script")).toString();
    if (!scriptName.isEmpty()) {
        QFileInfo fi(fileName);
        d->m_scriptName = fi.path() + QLatin1Char('/') + scriptName;
    }

    if (!d->m_scriptName.isEmpty() && !QFile::exists(d->m_scriptName)) {
        qCWarning(Log) << "Script file not found:" << d->m_scriptName;
        return false;
    }
    d->m_scriptFunction = obj.value(QLatin1String("function")).toString(QStringLiteral("main"));

    return !d->m_filters.empty() && !d->m_mimeType.isEmpty();
}

QJsonObject Extractor::toJson() const
{
    QJsonObject obj;
    obj.insert(QLatin1String("mimeType"), d->m_mimeType);

    QFileInfo metaFi(d->m_fileName);
    QFileInfo scriptFi(d->m_scriptName);
    if (metaFi.canonicalPath() == scriptFi.canonicalPath()) {
        obj.insert(QStringLiteral("script"), scriptFi.fileName());
    } else {
        obj.insert(QStringLiteral("script"), d->m_scriptName);
    }
    obj.insert(QStringLiteral("function"), d->m_scriptFunction);

    QJsonArray filters;
    std::transform(d->m_filters.begin(), d->m_filters.end(), std::back_inserter(filters), std::mem_fn(&ExtractorFilter::toJson));
    obj.insert(QStringLiteral("filter"), filters);

    return obj;
}

QString Extractor::name() const
{
    QFileInfo fi(d->m_fileName);
    if (d->m_index < 0) {
        return fi.baseName();
    }
    return fi.baseName() + QLatin1Char(':') + QString::number(d->m_index);
}

ExtractorInput::Type Extractor::type() const
{
    return ExtractorInput::typeFromMimeType(d->m_mimeType);
}

void Extractor::setType(ExtractorInput::Type type)
{
    setMimeType(ExtractorInput::typeToMimeType(type));
}

QString Extractor::mimeType() const
{
    return d->m_mimeType;
}

void Extractor::setMimeType(const QString &mimeType)
{
    d.detach();
    d->m_mimeType = mimeType;
}

QString Extractor::scriptFileName() const
{
    return d->m_scriptName;
}

void Extractor::setScriptFileName(const QString &script)
{
    d.detach();
    d->m_scriptName = script;
}

QString Extractor::scriptFunction() const
{
    return d->m_scriptFunction;
}

void Extractor::setScriptFunction(const QString &func)
{
    d.detach();
    d->m_scriptFunction = func;
}

const std::vector<ExtractorFilter> &Extractor::filters() const
{
    return d->m_filters;
}

void Extractor::setFilters(std::vector<ExtractorFilter> filters)
{
    d.detach();
    d->m_filters = std::move(filters);
}

QString Extractor::fileName() const
{
    return d->m_fileName;
}
