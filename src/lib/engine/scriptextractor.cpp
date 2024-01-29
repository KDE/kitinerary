/*
   SPDX-FileCopyrightText: 2017-2021 Volker Krause <vkrause@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "scriptextractor.h"
#include "extractorscriptengine_p.h"
#include "logging.h"

#include <KItinerary/ExtractorDocumentNode>
#include <KItinerary/ExtractorEngine>
#include <KItinerary/ExtractorFilter>
#include <KItinerary/ExtractorResult>

#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonObject>

using namespace KItinerary;

namespace KItinerary {
class ScriptExtractorPrivate
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

ScriptExtractor::ScriptExtractor()
    : d(std::make_unique<ScriptExtractorPrivate>())
{
}

ScriptExtractor::~ScriptExtractor() = default;

bool ScriptExtractor::load(const QJsonObject &obj, const QString &fileName, int index)
{
    d->m_fileName = fileName;
    d->m_index = index;

    d->m_mimeType = obj.value(QLatin1StringView("mimeType")).toString();

    const auto filterArray = obj.value(QLatin1StringView("filter")).toArray();
    for (const auto &filterValue : filterArray) {
        ExtractorFilter f;
        if (!f.load(filterValue.toObject())) {
            qCDebug(Log) << "invalid filter expression:" << fileName;
            return false;
        }
        d->m_filters.push_back(std::move(f));
    }

    const auto scriptName = obj.value(QLatin1StringView("script")).toString();
    if (!scriptName.isEmpty()) {
        QFileInfo fi(fileName);
        d->m_scriptName = fi.path() + QLatin1Char('/') + scriptName;
    }

    if (!d->m_scriptName.isEmpty() && !QFile::exists(d->m_scriptName)) {
        qCWarning(Log) << "Script file not found:" << d->m_scriptName;
        return false;
    }
    d->m_scriptFunction = obj.value(QLatin1StringView("function"))
                              .toString(QStringLiteral("main"));

    return !d->m_filters.empty() && !d->m_mimeType.isEmpty();
}

QJsonObject ScriptExtractor::toJson() const
{
    QJsonObject obj;
    obj.insert(QStringLiteral("mimeType"), d->m_mimeType);

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

QString ScriptExtractor::name() const
{
    QFileInfo fi(d->m_fileName);
    if (d->m_index < 0) {
        return fi.baseName();
    }
    return fi.baseName() + QLatin1Char(':') + QString::number(d->m_index);
}

QString ScriptExtractor::mimeType() const
{
    return d->m_mimeType;
}

void ScriptExtractor::setMimeType(const QString &mimeType)
{
    d->m_mimeType = mimeType;
}

QString ScriptExtractor::scriptFileName() const
{
    return d->m_scriptName;
}

void ScriptExtractor::setScriptFileName(const QString &script)
{
    d->m_scriptName = script;
}

QString ScriptExtractor::scriptFunction() const
{
    return d->m_scriptFunction;
}

void ScriptExtractor::setScriptFunction(const QString &func)
{
    d->m_scriptFunction = func;
}

QString ScriptExtractor::fileName() const
{
    return d->m_fileName;
}

const std::vector<ExtractorFilter>& ScriptExtractor::filters() const
{
    return d->m_filters;
}

void ScriptExtractor::setFilters(std::vector<ExtractorFilter> &&filters)
{
    d->m_filters = std::move(filters);
}

void ScriptExtractor::setFilters(const std::vector<ExtractorFilter> &filters)
{
    d->m_filters = filters;
}

bool ScriptExtractor::canHandle(const ExtractorDocumentNode &node) const
{
    if (node.mimeType() != d->m_mimeType) {
        return false;
    }

    // no filters matches always
    if (d->m_filters.empty()) {
        return true;
    }

    return std::any_of(d->m_filters.begin(), d->m_filters.end(), [&node](const auto &filter) {
        return filter.matches(node);
    });
}

ExtractorResult ScriptExtractor::extract(const ExtractorDocumentNode &node, const ExtractorEngine *engine) const
{
    std::vector<ExtractorDocumentNode> triggerNodes;
    for (const auto &filter : d->m_filters) {
        if (filter.scope() != ExtractorFilter::Current) {
            filter.allMatches(node, triggerNodes);
        }
    }

    if (triggerNodes.empty()) {
        return engine->scriptEngine()->execute(this, node, node);
    } else {
        ExtractorResult result;
        for (const auto &triggerNode : triggerNodes) {
            result.append(engine->scriptEngine()->execute(this, node, triggerNode));
        }
        return result;
    }
}
