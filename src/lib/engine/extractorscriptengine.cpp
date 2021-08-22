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
#include "jsapi/bytearray.h"
#include "jsapi/jsonld.h"

#include <QFile>
#include <QJSEngine>
#include <QJSValueIterator>
#include <QScopeGuard>
#include <QThread>
#include <QTimer>

using namespace KItinerary;

namespace KItinerary {
class ExtractorScriptEnginePrivate {
public:
    ~ExtractorScriptEnginePrivate();
    bool loadScript(const QString &fileName);

    JsApi::Barcode *m_barcodeApi = nullptr;
    JsApi::JsonLd *m_jsonLdApi = nullptr;
    QJSEngine m_engine;

    QThread m_watchdogThread;
    QTimer *m_watchdogTimer = nullptr;
};
}

ExtractorScriptEnginePrivate::~ExtractorScriptEnginePrivate()
{
    m_watchdogTimer->deleteLater();
    m_watchdogThread.quit();
    m_watchdogThread.wait();
}

ExtractorScriptEngine::ExtractorScriptEngine() = default;
ExtractorScriptEngine::~ExtractorScriptEngine() = default;

void ExtractorScriptEngine::ensureInitialized()
{
    if (d) {
        return;
    }

    d = std::make_unique<ExtractorScriptEnginePrivate>();
    d->m_engine.installExtensions(QJSEngine::ConsoleExtension);
    d->m_jsonLdApi = new JsApi::JsonLd(&d->m_engine);
    d->m_engine.globalObject().setProperty(QStringLiteral("JsonLd"), d->m_engine.newQObject(d->m_jsonLdApi));
    d->m_barcodeApi = new JsApi::Barcode;
    d->m_engine.globalObject().setProperty(QStringLiteral("Barcode"), d->m_engine.newQObject(d->m_barcodeApi));
    d->m_engine.globalObject().setProperty(QStringLiteral("ByteArray"), d->m_engine.newQObject(new JsApi::ByteArray));

    d->m_watchdogThread.start();
    d->m_watchdogTimer = new QTimer;
    d->m_watchdogTimer->setInterval(std::chrono::seconds(1));
    d->m_watchdogTimer->setSingleShot(true);
    d->m_watchdogTimer->moveToThread(&d->m_watchdogThread);
    QObject::connect(d->m_watchdogTimer, &QTimer::timeout, &d->m_engine, [this]() { d->m_engine.setInterrupted(true); }, Qt::DirectConnection);
}

void ExtractorScriptEngine::setBarcodeDecoder(BarcodeDecoder *barcodeDecoder)
{
    ensureInitialized();
    d->m_barcodeApi->setDecoder(barcodeDecoder);
}

// produce the same output as the JS engine error result fileName property would have
static QString fileNameToUrl(const QString &fileName)
{
    if (fileName.startsWith(QLatin1Char(':'))) {
        return QLatin1String("qrc:/") + QStringView(fileName).mid(1);
    }
    return QUrl::fromLocalFile(fileName).toString();
}

static void printScriptError(const QJSValue &result, const QString &fileNameFallback)
{
    const auto fileName = result.property(QStringLiteral("fileName"));
    // don't change the formatting without adjusting KItinerary Workbench too!
    qCWarning(Log).noquote().nospace() << "JS ERROR: [" << (fileName.isString() ? fileName.toString() : fileNameToUrl(fileNameFallback))
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
        printScriptError(result, fileName);
        return false;
    }

    return true;
}

ExtractorResult ExtractorScriptEngine::execute(const ScriptExtractor *extractor, const ExtractorDocumentNode &node, const ExtractorDocumentNode &triggerNode) const
{
    const_cast<ExtractorScriptEngine*>(this)->ensureInitialized();

    // watchdog setup
    QMetaObject::invokeMethod(d->m_watchdogTimer, qOverload<>(&QTimer::start));
    const auto watchdogStop = qScopeGuard([this]() {
        QMetaObject::invokeMethod(d->m_watchdogTimer, qOverload<>(&QTimer::stop));
    });
    d->m_engine.setInterrupted(false);

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

    d->m_jsonLdApi->setContextDate(node.contextDateTime());

    const auto nodeArg = d->m_engine.toScriptValue(node);
    const auto dataArg = nodeArg.property(QLatin1String("content"));
    const auto triggerArg = d->m_engine.toScriptValue(triggerNode);
    QJSValueList args{ dataArg, nodeArg, triggerArg };

    const auto result = mainFunc.call(args);
    if (result.isError()) {
        printScriptError(result, extractor->scriptFileName());
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
