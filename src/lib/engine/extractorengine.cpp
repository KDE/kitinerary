/*
   SPDX-FileCopyrightText: 2017-2021 Volker Krause <vkrause@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "config-kitinerary.h"
#include "extractorengine.h"

#include "barcodedecoder.h"
#include "abstractextractor.h"
#include "extractordocumentnode.h"
#include "extractordocumentnodefactory.h"
#include "extractordocumentprocessor.h"
#include "extractorresult.h"
#include "extractorrepository.h"
#include "extractorscriptengine_p.h"
#include "jsonlddocument.h"
#include "logging.h"

#include <KMime/Content>
#include <KMime/Message>

#include <QDateTime>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLocale>

#include <cstring>
#include <memory>

using namespace KItinerary;

namespace KItinerary {

class ExtractorEnginePrivate {
public:
    void processNode(ExtractorDocumentNode &node);

    ExtractorEngine *q = nullptr;
    std::vector<const AbstractExtractor*> m_additionalExtractors;
    ExtractorDocumentNode m_rootNode;
    ExtractorDocumentNode m_contextNode;
    ExtractorDocumentNodeFactory m_nodeFactory;
    ExtractorRepository m_repo;
    BarcodeDecoder m_barcodeDecoder;
    QString m_usedExtractor;
    ExtractorScriptEngine m_scriptEngine;
};

}

void ExtractorEnginePrivate::processNode(ExtractorDocumentNode& node)
{
    if (node.isNull()) {
        return;
    }

    node.processor()->expandNode(node, q);
    for (auto c : node.childNodes()) {
        processNode(c);
    }
    node.processor()->reduceNode(node);

    node.processor()->preExtract(node, q);
    std::vector<const AbstractExtractor*> extractors = m_additionalExtractors;
    m_repo.extractorsForNode(node, extractors);

    for (const auto &extractor : extractors) {
        auto res = extractor->extract(node, q);
        if (!res.isEmpty()) {
            m_usedExtractor = extractor->name();
            node.setResult(std::move(res));
        }
    }

    node.processor()->postExtract(node);

    // set modification time for all results that don't have it yet
    if (node.contextDateTime().isValid()) {
        auto result = node.result().jsonLdResult();
        for (int i = 0; i < result.size(); ++i) {
            auto res = result.at(i).toObject();
            if (!res.contains(QLatin1String("modifiedTime"))) {
                res.insert(QStringLiteral("modifiedTime"), node.contextDateTime().toString(Qt::ISODate));
            }
            result[i] = res;
        }
        node.setResult(result);
    }
}


ExtractorEngine::ExtractorEngine()
    : d(new ExtractorEnginePrivate)
{
    d->q = this;
}

ExtractorEngine::ExtractorEngine(ExtractorEngine &&) noexcept = default;

ExtractorEngine::~ExtractorEngine()
{
    // ensure we destroy nodes before we destroy the node factory
    clear();
}

void ExtractorEngine::clear()
{
    d->m_rootNode = {};
    d->m_contextNode = {};
}

void ExtractorEngine::setData(const QByteArray &data, QStringView fileName, QStringView mimeType)
{
    d->m_rootNode = d->m_nodeFactory.createNode(data, fileName, mimeType);
}

void ExtractorEngine::setContent(const QVariant &data, QStringView mimeType)
{
    d->m_rootNode = d->m_nodeFactory.createNode(data, mimeType);
}

void ExtractorEngine::setContext(const QVariant &data, QStringView mimeType)
{
    d->m_contextNode = d->m_nodeFactory.createNode(data, mimeType);
}

void ExtractorEngine::setContextDate(const QDateTime &dt)
{
    d->m_contextNode.setContextDateTime(dt);
}

QJsonArray ExtractorEngine::extract()
{
    d->m_rootNode.setParent(d->m_contextNode);
    d->processNode(d->m_rootNode);
    return d->m_rootNode.result().jsonLdResult();
}

void ExtractorEngine::setUseSeparateProcess(bool separateProcess)
{
    d->m_nodeFactory.setUseSeparateProcess(separateProcess);
}

void ExtractorEngine::setAdditionalExtractors(std::vector<const AbstractExtractor*> &&extractors)
{
    d->m_additionalExtractors = std::move(extractors);
}

QString ExtractorEngine::usedCustomExtractor() const
{
    return d->m_usedExtractor;
}

const ExtractorDocumentNodeFactory* ExtractorEngine::documentNodeFactory() const
{
    return &d->m_nodeFactory;
}

const BarcodeDecoder* ExtractorEngine::barcodeDecoder() const
{
    return &d->m_barcodeDecoder;
}

const ExtractorRepository* ExtractorEngine::extractorRepository() const
{
    return &d->m_repo;
}

const ExtractorScriptEngine* ExtractorEngine::scriptEngine() const
{
    d->m_scriptEngine.setBarcodeDecoder(&d->m_barcodeDecoder);
    return &d->m_scriptEngine;
}

ExtractorDocumentNode ExtractorEngine::rootDocumentNode() const
{
    return d->m_rootNode;
}