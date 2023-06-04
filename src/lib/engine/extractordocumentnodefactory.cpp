/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "extractordocumentnodefactory.h"
#include "extractordocumentnode.h"
#include "extractordocumentprocessor.h"
#include "logging.h"

#include "processors/binarydocumentprocessor.h"
#include "processors/eradocumentprocessor.h"
#include "processors/externalprocessor.h"
#include "processors/htmldocumentprocessor.h"
#include "processors/httpresponseprocessor.h"
#include "processors/iatabcbpdocumentprocessor.h"
#include "processors/icaldocumentprocessor.h"
#include "processors/imagedocumentprocessor.h"
#include "processors/jsonlddocumentprocessor.h"
#include "processors/mimedocumentprocessor.h"
#include "processors/pdfdocumentprocessor.h"
#include "processors/pkpassdocumentprocessor.h"
#include "processors/plistdocumentprocessor.h"
#include "processors/textdocumentprocessor.h"
#include "processors/uic9183documentprocessor.h"
#include "processors/vdvdocumentprocessor.h"

#include <QHash>
#include <QMimeDatabase>

using namespace KItinerary;

enum {
    MinDocumentSize = 4,
    MaxDocumentSize = 4000000,
};

namespace KItinerary {
class ExtractorDocumentNodeFactoryStatic {
public:
    ExtractorDocumentNodeFactoryStatic();

    void registerProcessor(std::unique_ptr<ExtractorDocumentProcessor> &&processor, QStringView canonicalMimeType,
                           std::initializer_list<QStringView> aliasMimeTypes = {}, QStringView fallbackMimeType = {});

    template <typename T>
    inline void registerProcessor(QStringView canonicalMimeType, std::initializer_list<QStringView> aliasMimeTypes = {}, QStringView fallbackMimeType = {})
    {
        registerProcessor(std::make_unique<T>(), canonicalMimeType, aliasMimeTypes, fallbackMimeType);
    }

    void registerBuiltIn();
    QStringView resolveAlias(QStringView mimeType) const;

    struct ProcessorData {
        QString mimeType;
        const ExtractorDocumentProcessor* processor;
    };
    std::vector<ProcessorData> m_probeProcessors;
    std::vector<ProcessorData> m_fallbackProbeProcessors;
    std::vector<ProcessorData> m_mimetypeProcessorMap;
    QHash<QString, QString> m_aliasMap;

    // just for memory management
    std::vector<std::unique_ptr<ExtractorDocumentProcessor>> processorPool;

    static void insertProcessor(const ExtractorDocumentProcessor *proc, QStringView mimeType, std::vector<ProcessorData> &procMap);
};

class ExtractorDocumentNodeFactoryPrivate {
public:
    ExtractorDocumentNodeFactoryStatic *s;
    std::unique_ptr<ExtractorDocumentProcessor> interceptProcessor;
};
}

ExtractorDocumentNodeFactoryStatic::ExtractorDocumentNodeFactoryStatic()
{
    registerBuiltIn();
}

void ExtractorDocumentNodeFactoryStatic::insertProcessor(const ExtractorDocumentProcessor *proc, QStringView mimeType, std::vector<ProcessorData> &procMap)
{
    if (mimeType.empty()) {
        return;
    }

    const auto it = std::lower_bound(procMap.begin(), procMap.end(), mimeType, [](const auto &proc, auto mt) {
        return proc.mimeType < mt;
    });
    if (it != procMap.end() && (*it).mimeType == mimeType) {
        qCWarning(Log) << "Document processor already registered for mimetype:" << mimeType;
        return;
    }

    procMap.insert(it, { mimeType.toString(), proc });
}

void ExtractorDocumentNodeFactoryStatic::registerProcessor(std::unique_ptr<ExtractorDocumentProcessor> &&processor, QStringView canonicalMimeType,
                                                           std::initializer_list<QStringView> aliasMimeTypes, QStringView fallbackMimeType)
{
    insertProcessor(processor.get(), canonicalMimeType, m_probeProcessors);
    insertProcessor(processor.get(), canonicalMimeType, m_mimetypeProcessorMap);
    for (const auto mt : aliasMimeTypes) {
        m_aliasMap.insert(mt.toString(), canonicalMimeType.isEmpty() ? fallbackMimeType.toString() : canonicalMimeType.toString());
    }
    insertProcessor(processor.get(), fallbackMimeType, m_fallbackProbeProcessors);
    insertProcessor(processor.get(), fallbackMimeType, m_mimetypeProcessorMap);
    processorPool.push_back(std::move(processor));
}

void ExtractorDocumentNodeFactoryStatic::registerBuiltIn()
{
    registerProcessor<PdfDocumentProcessor>(u"application/pdf");
    registerProcessor<PkPassDocumentProcessor>(u"application/vnd.apple.pkpass");
    registerProcessor<IcalEventProcessor>(u"internal/event");
    registerProcessor<ImageDocumentProcessor>(u"internal/qimage", {u"image/png", u"image/jpeg"});
    registerProcessor<ElbDocumentProcessor>(u"internal/era-elb");
    registerProcessor<SsbDocumentProcessor>(u"internal/era-ssb");
    registerProcessor<IataBcbpDocumentProcessor>(u"internal/iata-bcbp");
    registerProcessor<Uic9183DocumentProcessor>(u"internal/uic9183");
    registerProcessor<VdvDocumentProcessor>(u"internal/vdv");
    registerProcessor<IcalCalendarProcessor>(u"text/calendar");
    registerProcessor<PListDocumentProcessor>(u"application/x-plist");
    registerProcessor<HttpResponseProcessor>(u"internal/http-response");
    registerProcessor<HarDocumentProcessor>(u"internal/har-archive");

    // fallback types that catch a very broad set of input types
    // order matters particularly here, the broadest ones need to go last
    registerProcessor<JsonLdDocumentProcessor>({}, {u"application/json"}, u"application/ld+json");
    registerProcessor<MimeDocumentProcessor>({}, {u"application/mbox"}, u"message/rfc822");
    registerProcessor<HtmlDocumentProcessor>({}, {u"application/xhtml+xml"}, u"text/html");
    registerProcessor<TextDocumentProcessor>({}, {}, u"text/plain");
    registerProcessor<BinaryDocumentProcessor>({}, {}, u"application/octet-stream");
}

QStringView ExtractorDocumentNodeFactoryStatic::resolveAlias(QStringView mimeType) const
{
    const auto it = m_aliasMap.find(mimeType.toString());
    if (it != m_aliasMap.end()) {
        return it.value();
    }
    return mimeType;
}


ExtractorDocumentNodeFactory::ExtractorDocumentNodeFactory()
    : d(std::make_unique<ExtractorDocumentNodeFactoryPrivate>())
{
    static ExtractorDocumentNodeFactoryStatic s_factory;
    d->s = &s_factory;
}

ExtractorDocumentNodeFactory::~ExtractorDocumentNodeFactory() = default;

ExtractorDocumentNode ExtractorDocumentNodeFactory::createNode(const QByteArray &data, QStringView fileName, QStringView mimeType) const
{
    if (data.size() <= MinDocumentSize || data.size() > MaxDocumentSize) {
        return {};
    }

    if (d->interceptProcessor && d->interceptProcessor->canHandleData(data, fileName)) {
        auto node = d->interceptProcessor->createNodeFromData(data);
        if (node.mimeType().isEmpty()) {
            node.setMimeType(QStringLiteral("internal/external-process"));
        }
        node.setProcessor(d->interceptProcessor.get());
        return node;
    }

    QString autoDetectedMimeType;
    if (mimeType.isEmpty()) {
        // let processors check themselves if they support this data
        for (const auto &p : d->s->m_probeProcessors) {
            if (p.processor->canHandleData(data, fileName)) {
                auto node = p.processor->createNodeFromData(data);
                if (node.content().isNull()) {
                    continue;
                }

                node.setMimeType(p.mimeType);
                node.setProcessor(p.processor);
                return node;
            }
        }
        // same again with the basic types that ultimately will accept anything
        for (const auto &p : d->s->m_fallbackProbeProcessors) {
            if (p.processor->canHandleData(data, fileName)) {
                auto node = p.processor->createNodeFromData(data);
                if (node.content().isNull()) {
                    continue;
                }

                node.setMimeType(p.mimeType);
                node.setProcessor(p.processor);
                return node;
            }
        }

        // if none felt responsible, try the generic mimetype detection
        QMimeDatabase db;
        if (fileName.isEmpty()) {
            autoDetectedMimeType = db.mimeTypeForData(data).name();
        } else {
            autoDetectedMimeType = db.mimeTypeForFileNameAndData(fileName.toString(), data).name();
        }
        mimeType = autoDetectedMimeType;
    }

    mimeType = d->s->resolveAlias(mimeType);
    const auto it = std::lower_bound(d->s->m_mimetypeProcessorMap.begin(), d->s->m_mimetypeProcessorMap.end(), mimeType, [](const auto &proc, auto mt) {
        return proc.mimeType < mt;
    });
    if (it == d->s->m_mimetypeProcessorMap.end() || (*it).mimeType != mimeType) {
        qCDebug(Log) << "No document processor found for mimetype" << mimeType;
        return {};
    }

    auto node = (*it).processor->createNodeFromData(data);
    node.setMimeType((*it).mimeType);
    node.setProcessor((*it).processor);
    return node;
}

ExtractorDocumentNode ExtractorDocumentNodeFactory::createNode(const QVariant &decodedData, QStringView mimeType) const
{
    mimeType = d->s->resolveAlias(mimeType);
    const auto it = std::lower_bound(d->s->m_mimetypeProcessorMap.begin(), d->s->m_mimetypeProcessorMap.end(), mimeType, [](const auto &proc, auto mt) {
        return proc.mimeType < mt;
    });
    if (it == d->s->m_mimetypeProcessorMap.end() || (*it).mimeType != mimeType) {
        qCDebug(Log) << "No document processor found for mimetype" << mimeType;
        return {};
    }

    auto node = (*it).processor->createNodeFromContent(decodedData);
    node.setMimeType((*it).mimeType);
    node.setProcessor((*it).processor);
    return node;
}

void ExtractorDocumentNodeFactory::registerProcessor(std::unique_ptr<ExtractorDocumentProcessor> &&processor, QStringView mimeType,
                                                     std::initializer_list<QStringView> aliasMimeTypes)
{
    d->s->registerProcessor(std::move(processor), mimeType, aliasMimeTypes);
}

void ExtractorDocumentNodeFactory::setUseSeparateProcess(bool separateProcess)
{
    if (separateProcess && !d->interceptProcessor) {
        d->interceptProcessor = std::make_unique<ExternalProcessor>();
    } else if (!separateProcess && d->interceptProcessor) {
        d->interceptProcessor.reset();
    }
}
