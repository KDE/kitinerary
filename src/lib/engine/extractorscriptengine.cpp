/*
   SPDX-FileCopyrightText: 2017-2021 Volker Krause <vkrause@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "extractorscriptengine_p.h"
#include "extractordocumentnode.h"
#include "extractordocumentprocessor.h"
#include "extractorresult.h"
#include "scriptextractor.h"
#include "logging.h"

#include "jsapi/barcode.h"
#include "jsapi/context.h"
#include "jsapi/jsonld.h"

#include <KItinerary/Uic9183Parser>
#include <KItinerary/VdvTicket>

#include <QFile>
#include <QJSEngine>
#include <QJSValueIterator>
#include <QScopeGuard>

using namespace KItinerary;

namespace KItinerary {
class ExtractorScriptEnginePrivate {
public:
    bool loadScript(const QString &fileName);

    JsApi::Barcode *m_barcodeApi = nullptr;
    JsApi::Context *m_context = nullptr;
    JsApi::JsonLd *m_jsonLdApi = nullptr;
    QJSEngine m_engine;
};
}

ExtractorScriptEngine::ExtractorScriptEngine() = default;
ExtractorScriptEngine::~ExtractorScriptEngine() = default;

void ExtractorScriptEngine::ensureInitialized()
{
    if (d) {
        return;
    }

    d = std::make_unique<ExtractorScriptEnginePrivate>();
    d->m_context = new JsApi::Context; // will be deleted by QJSEngine taking ownership
    d->m_engine.installExtensions(QJSEngine::ConsoleExtension);
    d->m_jsonLdApi = new JsApi::JsonLd(&d->m_engine);
    d->m_engine.globalObject().setProperty(QStringLiteral("JsonLd"), d->m_engine.newQObject(d->m_jsonLdApi));
    d->m_barcodeApi = new JsApi::Barcode;
    d->m_engine.globalObject().setProperty(QStringLiteral("Barcode"), d->m_engine.newQObject(d->m_barcodeApi));
    d->m_engine.globalObject().setProperty(QStringLiteral("Context"), d->m_engine.newQObject(d->m_context));
}

void ExtractorScriptEngine::setBarcodeDecoder(BarcodeDecoder *barcodeDecoder)
{
    ensureInitialized();
    d->m_barcodeApi->setDecoder(barcodeDecoder);
}

static void printScriptError(const QJSValue &result)
{
    // don't change the formatting without adjusting KItinerary Workbench too!
    qCWarning(Log).noquote().nospace() << "JS ERROR: [" << result.property(QStringLiteral("fileName")).toString()
        << "]:" << result.property(QStringLiteral("lineNumber")).toInt() << ": " << result.toString();
}

bool ExtractorScriptEnginePrivate::loadScript(const QString &fileName)
{
    // TODO we could skip this is if the right script is already loaded
    // we cannot do this unconditionally however without breaking KItinerary Workbench's live editing
    if (fileName.isEmpty()) {
        return false;
    }

    QFile f(fileName);
    if (!f.open(QFile::ReadOnly)) {
        qCWarning(Log) << "Failed to open extractor script" << f.fileName() << f.errorString();
        return false;
    }

    auto result = m_engine.evaluate(QString::fromUtf8(f.readAll()), f.fileName());
    if (result.isError()) {
        printScriptError(result);
        return false;
    }

    return true;
}

ExtractorResult ExtractorScriptEngine::execute(const ScriptExtractor *extractor, const ExtractorDocumentNode &node, const ExtractorDocumentNode &triggerNode) const
{
    const_cast<ExtractorScriptEngine*>(this)->ensureInitialized();

    if (!d->loadScript(extractor->scriptFileName())) {
        return {};
    }

    auto mainFunc = d->m_engine.globalObject().property(extractor->scriptFunction());
    if (!mainFunc.isCallable()) {
        qCWarning(Log) << "Script entry point not found!" << extractor->scriptFunction();
        return {};
    }

    qCDebug(Log) << "Running script extractor" << extractor->scriptFileName() << extractor->scriptFunction();
    node.setScriptEngine(&d->m_engine);
    triggerNode.setScriptEngine(&d->m_engine);
    const auto engineReset = qScopeGuard([&node, &triggerNode]{
        node.setScriptEngine(nullptr);
        triggerNode.setScriptEngine(nullptr);
    });

    // ### legacy context API, replace that by passing trigger node as a third argument eventually
    if (triggerNode.result().isEmpty()) {
        d->m_context->m_data = {};
    } else {
        d->m_context->m_data = d->m_engine.toScriptValue(triggerNode.result().jsonLdResult());
    }

    if (triggerNode.isA<Uic9183Parser>()) {
        d->m_context->m_barcode = triggerNode.content<Uic9183Parser>().rawData();
    } else if (triggerNode.isA<VdvTicket>()) {
        d->m_context->m_barcode = triggerNode.content<VdvTicket>().rawData();
    } else if (triggerNode.isA<QByteArray>()) {
        d->m_context->m_barcode = triggerNode.content<QByteArray>();
    } else if (triggerNode.isA<QString>()) {
        d->m_context->m_barcode = triggerNode.content().toString();
    } else {
        d->m_context->m_barcode.clear();
    }

    if (triggerNode.location().isNull()) {
        d->m_context->m_pdfPageNum = -1;
    } else {
        d->m_context->m_pdfPageNum = triggerNode.location().toInt();
    }

    d->m_context->m_senderDate = node.contextDateTime();

    d->m_jsonLdApi->setContextDate(node.contextDateTime());
    d->m_barcodeApi->setContextDate(node.contextDateTime());

    const auto nodeArg = d->m_engine.toScriptValue(node);
    const auto dataArg = nodeArg.property(QLatin1String("content"));
    const auto triggerArg = d->m_engine.toScriptValue(triggerNode);
    QJSValueList args{ dataArg, nodeArg, triggerArg };

    const auto result = mainFunc.call(args);
    if (result.isError()) {
        printScriptError(result);
        return {};
    }

    QJsonArray out;
    if (result.isArray()) {
        QJSValueIterator it(result);
        while (it.hasNext()) {
            it.next();
            if (it.value().isObject()) {
                out.push_back(QJsonValue::fromVariant(it.value().toVariant()));
            }
        }
    } else if (result.isObject()) {
        out.push_back(QJsonValue::fromVariant(result.toVariant()));
    } else {
        qCWarning(Log) << "Invalid result type from script";
    }

    return out;
}
