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

#include "config-kitinerary.h"
#include "barcodedecoder.h"
#include "extractorengine.h"
#include "extractor.h"
#include "extractorinput.h"
#include "extractorrepository.h"
#include "generic/genericpdfextractor_p.h"
#include "generic/genericpkpassextractor_p.h"
#include "generic/genericuic918extractor_p.h"
#include "generic/genericvdvextractor_p.h"
#include "htmldocument.h"
#include "iatabcbpparser.h"
#include "jsonlddocument.h"
#include "logging.h"
#include "pdf/pdfdocument.h"
#include "generic/structureddataextractor_p.h"
#include "uic9183/uic9183parser.h"
#include "vdv/vdvticketparser.h"

#include "jsapi/barcode.h"
#include "jsapi/context.h"
#include "jsapi/jsonld.h"

#ifdef HAVE_KCAL
#include <KCalendarCore/MemoryCalendar>
#include <KCalendarCore/Event>
#include <KCalendarCore/ICalFormat>
#endif

#include <KPkPass/Pass>

#include <KMime/Content>
#include <KMime/Message>

#include <QDateTime>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLocale>
#include <QJSEngine>
#include <QJSValueIterator>
#include <QProcess>

#include <cstring>

using namespace KItinerary;

namespace KItinerary {

class ExtractorEnginePrivate {
public:
    void setupEngine();
    void resetContent();

    void openDocument();
    bool shouldExtractExternally() const;
    void extractExternal();

    void extractRecursive(KMime::Content *content);
    void extractDocument();
    void extractGeneric();
    void extractCustom();
    void extractCustomForGenericResults();

    void determineExtractors();

    void executeScript(const Extractor &extractor);
    void processScriptResult(const QJSValue &result);

    void setContext(PdfDocument *pdf);

    ExtractorEngine *q = nullptr;
    std::vector<Extractor> m_extractors;
    std::vector<Extractor> m_additionalExtractors;
    JsApi::Barcode *m_barcodeApi = nullptr;
    JsApi::Context *m_context = nullptr;
    JsApi::JsonLd *m_jsonLdApi = nullptr;
    QString m_text;
    QByteArray m_data;
    ExtractorInput::Type m_inputType = ExtractorInput::Unknown;
    std::unique_ptr<HtmlDocument, std::function<void(HtmlDocument*)>> m_htmlDoc;
    std::unique_ptr<PdfDocument, std::function<void(PdfDocument*)>> m_pdfDoc;
    std::unique_ptr<KPkPass::Pass, std::function<void(KPkPass::Pass*)>> m_pass;
#ifdef HAVE_KCAL
    KCalendarCore::Calendar::Ptr m_calendar;
#endif
    KMime::Content *m_mimeContent = nullptr;
    KMime::Content *m_mimeContext = nullptr;
    std::unique_ptr<KMime::Content> m_ownedMimeContent;
    GenericPdfExtractor m_genericPdfExtractor;
    std::vector<GenericExtractor::Result> m_genericResults;
    QJsonArray m_result;
    QJSEngine m_engine;
    ExtractorRepository m_repo;
    BarcodeDecoder m_barcodeDecoder;
    QString m_externalExtractor;
    QString m_usedExtractor;
};

template <typename T>
static std::unique_ptr<T, std::function<void(T*)>> make_owning_ptr(T *ptr)
{
    return std::unique_ptr<T, std::function<void(T*)>>(ptr, [](T *ptr){ delete ptr; });
}

template <typename T>
static std::unique_ptr<T, std::function<void(T*)>> make_nonowning_ptr(T *ptr)
{
    return std::unique_ptr<T, std::function<void(T*)>>(ptr, [](T*){});
}

}

void ExtractorEnginePrivate::setupEngine()
{
    m_context = new JsApi::Context; // will be deleted by QJSEngine taking ownership
    m_engine.installExtensions(QJSEngine::ConsoleExtension);
    m_jsonLdApi = new JsApi::JsonLd(&m_engine);
    m_engine.globalObject().setProperty(QStringLiteral("JsonLd"), m_engine.newQObject(m_jsonLdApi));
    m_barcodeApi = new JsApi::Barcode;
    m_barcodeApi->setDecoder(&m_barcodeDecoder);
    m_engine.globalObject().setProperty(QStringLiteral("Barcode"), m_engine.newQObject(m_barcodeApi));
    m_engine.globalObject().setProperty(QStringLiteral("Context"), m_engine.newQObject(m_context));
}

bool ExtractorEnginePrivate::shouldExtractExternally() const
{
    return !m_externalExtractor.isEmpty() && !m_data.isEmpty() && m_inputType == ExtractorInput::Pdf;
}

void ExtractorEnginePrivate::extractExternal()
{
    m_extractors.clear();
    if (m_mimeContext) {
        m_repo.extractorsForMessage(m_mimeContext, m_extractors);
    }
    QStringList extNames;
    extNames.reserve(m_extractors.size());
    std::transform(m_extractors.begin(), m_extractors.end(), std::back_inserter(extNames), [](const auto &ext) { return ext.name(); });

    QProcess proc;
    proc.setProgram(m_externalExtractor);

    QStringList args({QLatin1String("--type"), ExtractorInput::typeToString(m_inputType),
                      QLatin1String("--context-date"), m_context->m_senderDate.toString(Qt::ISODate),
                      QLatin1String("--extractors"), extNames.join(QLatin1Char(';'))});
    const auto extraPaths = m_repo.additionalSearchPaths();
    for (const auto &p : extraPaths) {
        args.push_back(QStringLiteral("--additional-search-path"));
        args.push_back(p);
    }

    proc.setArguments(args);
    proc.start(QProcess::ReadWrite);
    proc.setProcessChannelMode(QProcess::ForwardedErrorChannel);
    if (!proc.waitForStarted(1000)) {
        qCWarning(Log) << "could not start external extractor" << m_externalExtractor << proc.errorString();
        return;
    }
    proc.write(m_data);
    proc.closeWriteChannel();
    if (!proc.waitForFinished(15000)) {
        qCWarning(Log) << "external extractor did not exit cleanly" << m_externalExtractor << proc.errorString();
        return;
    }

    const auto res = QJsonDocument::fromJson(proc.readAllStandardOutput()).array();
    std::copy(res.begin(), res.end(), std::back_inserter(m_result));
}


ExtractorEngine::ExtractorEngine()
    : d(new ExtractorEnginePrivate)
{
    d->q = this;
    d->m_genericPdfExtractor.setBarcodeDecoder(&d->m_barcodeDecoder);
    d->setupEngine();
}

ExtractorEngine::ExtractorEngine(ExtractorEngine &&) noexcept = default;
ExtractorEngine::~ExtractorEngine() = default;

void ExtractorEngine::clear()
{
    d->resetContent();
    d->m_result = {};
    d->m_mimeContext = nullptr;
    d->m_context->m_senderDate = {};
    d->m_ownedMimeContent.reset();
    d->m_barcodeDecoder.clearCache();
    d->m_usedExtractor.clear();
}

void ExtractorEnginePrivate::resetContent()
{
    m_text.clear();
    m_data.clear();
    m_inputType = ExtractorInput::Unknown;
    m_pdfDoc.reset();
    m_htmlDoc.reset();
    m_pass.reset();
#ifdef HAVE_KCAL
    m_calendar.reset();
#endif
    m_mimeContent = nullptr;
}

void ExtractorEngine::setText(const QString &text)
{
    d->m_text = text;
}

void ExtractorEngine::setHtmlDocument(HtmlDocument *htmlDoc)
{
    d->m_htmlDoc = make_nonowning_ptr(htmlDoc);
}

void ExtractorEngine::setPdfDocument(PdfDocument *pdfDoc)
{
    d->m_pdfDoc = make_nonowning_ptr(pdfDoc);
    d->setContext(d->m_pdfDoc.get());
}

void ExtractorEngine::setPass(KPkPass::Pass *pass)
{
    d->m_pass = make_nonowning_ptr(pass);
}

void ExtractorEngine::setCalendar(const QSharedPointer<KCalendarCore::Calendar> &calendar)
{
#ifdef HAVE_KCAL
    d->m_calendar = calendar;
#else
    Q_UNUSED(calendar);
#endif
}

void ExtractorEngine::setData(const QByteArray &data, const QString &fileName)
{
    // let's not even try to parse anything with implausible size
    if (data.size() <= 4 || data.size() > 4000000) {
        return;
    }

    const auto nameType = ExtractorInput::typeFromFileName(fileName);
    const auto contentType = ExtractorInput::typeFromContent(data);
    setData(data, nameType == ExtractorInput::Unknown ? contentType : nameType);
}

void ExtractorEngine::setData(const QByteArray &data, ExtractorInput::Type type)
{
    // let's not even try to parse anything with implausible size
    if (data.size() <= 4 || data.size() > 4000000) {
        return;
    }

    d->m_data = data;
    d->m_inputType = type;
}

void ExtractorEnginePrivate::openDocument()
{
    if (m_data.isEmpty())  {
        return;
    }

    switch (m_inputType) {
        case ExtractorInput::PkPass:
            m_pass = make_owning_ptr(KPkPass::Pass::fromData(m_data));
            m_data.clear();
            break;
        case ExtractorInput::Pdf:
            m_pdfDoc = make_owning_ptr(PdfDocument::fromData(m_data));
            m_data.clear();
            if (m_pdfDoc) {
                setContext(m_pdfDoc.get());
            }
            break;
        case ExtractorInput::Html:
            m_htmlDoc = make_owning_ptr(HtmlDocument::fromData(m_data));
            m_data.clear();
            break;
        case ExtractorInput::ICal:
        {
#ifdef HAVE_KCAL
            m_calendar.reset(new KCalendarCore::MemoryCalendar(QTimeZone::systemTimeZone()));
            KCalendarCore::ICalFormat format;
            if (format.fromRawString(m_calendar, m_data)) {
                m_calendar->setProductId(format.loadedProductId());
                break;
            }
            qCDebug(Log) << "Failed to parse iCal content.";
            m_calendar.reset();
#else
            qCDebug(Log) << "Trying to exctract ical file, but ical support is not enabled.";
#endif
            m_data.clear();
            break;
        }
        case ExtractorInput::Text:
            m_text = QString::fromUtf8(m_data);
            m_data.clear();
            break;
        case ExtractorInput::Email:
            m_ownedMimeContent.reset(new KMime::Message);
            m_ownedMimeContent->setContent(KMime::CRLFtoLF(m_data));
            m_ownedMimeContent->parse();
            m_data.clear();
            q->setContent(m_ownedMimeContent.get());
            break;
        case ExtractorInput::JsonLd:
        {
            // pass through JSON data, so the using code can apply post-processing to that
            const auto doc = QJsonDocument::fromJson(m_data);
            if (doc.isObject()) {
                m_result.push_back(doc.object());
            } else if (doc.isArray()) {
                m_result = doc.array();
            }
            m_data.clear();
            break;
        }
        default:
            break;
    }
}

void ExtractorEngine::setContent(KMime::Content *content)
{
    setContext(content);

    auto mtType = ExtractorInput::Unknown;
    auto fnType = ExtractorInput::Unknown;
    const auto ct = content->contentType(false);
    if (ct) {
        mtType = ExtractorInput::typeFromMimeType(QString::fromLatin1(ct->mimeType()));
        fnType = ExtractorInput::typeFromFileName(ct->name());
    }
    const auto cd = content->contentDisposition(false);
    if (fnType == ExtractorInput::Unknown && cd) {
        fnType = ExtractorInput::typeFromFileName(cd->filename());
    }

    if (mtType == ExtractorInput::PkPass || fnType == ExtractorInput::PkPass) {
        setData(content->decodedContent(), ExtractorInput::PkPass);
    } else if (mtType == ExtractorInput::ICal || fnType == ExtractorInput::ICal) {
        setData(content->decodedContent(), ExtractorInput::ICal);
    } else if (mtType == ExtractorInput::Pdf || fnType == ExtractorInput::Pdf) {
        setData(content->decodedContent(), ExtractorInput::Pdf);
    } else if (mtType == ExtractorInput::Html) {
        setData(content->decodedContent(), ExtractorInput::Html);
    } else if ( (mtType == ExtractorInput::Text) || (!ct && content->isTopLevel())) {
        d->m_text = content->decodedText();
    }

    d->m_mimeContent = (ct && ct->isMultipart()) ? content : nullptr;
}

void ExtractorEngine::setContext(KMime::Content *context)
{
    d->m_mimeContext = context;
    if (context) {
        auto dateHdr = context->header<KMime::Headers::Date>();
        while (!dateHdr && context->parent()) {
            context = context->parent();
            dateHdr = context->header<KMime::Headers::Date>();
        }
        if (dateHdr) {
            setContextDate(dateHdr->dateTime());
            return;
        }
    }

    setContextDate({});
}

void ExtractorEnginePrivate::setContext(PdfDocument *pdf)
{
    auto dt = pdf->modificationTime();
    if (!dt.isValid()) {
        dt = pdf->creationTime();
    }
    if (dt.isValid() && dt.date().year() > 2000 && dt < QDateTime::currentDateTime()) {
        q->setContextDate(dt);
    }
}

void ExtractorEngine::setContextDate(const QDateTime &dt)
{
    d->m_context->m_senderDate = dt;
    d->m_jsonLdApi->setContextDate(dt);
    d->m_barcodeApi->setContextDate(dt);
    d->m_genericPdfExtractor.setContextDate(dt);
}

QJsonArray ExtractorEngine::extract()
{
    d->extractDocument();
    return d->m_result;
}

void ExtractorEnginePrivate::extractRecursive(KMime::Content *content)
{
    QJsonArray aggregatedResult;
    const auto children = content->contents();
    for (const auto child : children) {
        resetContent();
        q->setContent(child);
        extractDocument();

        // the extractor takes early exits if data has been found, so make it look like that isn't the case
        std::copy(m_result.begin(), m_result.end(), std::back_inserter(aggregatedResult));
        m_result = {};
    }

    m_result = std::move(aggregatedResult);
}

void ExtractorEnginePrivate::extractDocument()
{
    // recurse into email MIME nodes if needed
    if (m_inputType == ExtractorInput::Email) {
        openDocument();
    }
    if (m_mimeContent) {
        extractRecursive(m_mimeContent);
        return;
    }

    if (shouldExtractExternally()) {
        extractExternal();
        return;
    }
    openDocument();

    // generic extraction
    m_genericResults.clear();
    extractGeneric();

    // determine which custom extractos apply, and run them
    m_extractors.clear();
    determineExtractors();
    if (!m_genericResults.empty()) {
        // run the custom extractors for each generic extractor find and use that as its context
        extractCustomForGenericResults();
    } else {
        // run custom extractors without context
        extractCustom();
        // if we didn't find anything, let's use whatever the generic extractors found
        if (m_result.empty()) {
            for (const auto &gr : m_genericResults) {
                std::copy(gr.result.begin(), gr.result.end(), std::back_inserter(m_result));
            }
        }
    }
}

void ExtractorEnginePrivate::extractGeneric()
{
    if (m_htmlDoc) {
        const auto res = StructuredDataExtractor::extract(m_htmlDoc.get());
        m_genericResults.emplace_back(GenericExtractor::Result{res});
    }
    else if (m_pdfDoc) {
        m_genericResults = m_genericPdfExtractor.extract(m_pdfDoc.get());
    }
    else if (m_pass) {
        m_genericResults.emplace_back(GenericExtractor::Result{{GenericPkPassExtractor::extract(m_pass.get(), m_context->m_senderDate)}});
    }
    else  if (!m_text.isEmpty()) {
        if (IataBcbpParser::maybeIataBcbp(m_text)) {
            const auto res = IataBcbpParser::parse(m_text, m_context->m_senderDate.date());
            m_genericResults.emplace_back(GenericExtractor::Result{JsonLdDocument::toJson(res), m_text});
        }
    }
    else if (!m_data.isEmpty()) {
        if (Uic9183Parser::maybeUic9183(m_data)) {
            QJsonArray res;
            GenericUic918Extractor::extract(m_data, res, m_context->m_senderDate);
            m_genericResults.emplace_back(GenericExtractor::Result{res, m_data});
            return;
        } else if (VdvTicketParser::maybeVdvTicket(m_data)) {
            const auto res = GenericVdvExtractor::extract(m_data);
            if (!res.isEmpty()) {
                m_genericResults.emplace_back(GenericExtractor::Result(res, m_data));
            }
            return;
        }
        // try again as text
        m_text = QString::fromUtf8(m_data);
        extractGeneric();
    }

}

void ExtractorEnginePrivate::determineExtractors()
{
    m_extractors = m_additionalExtractors;

    if (m_pass) {
        m_repo.extractorsForPass(m_pass.get(), m_extractors);
#ifdef HAVE_KCAL
    } else if (m_calendar) {
        m_repo.extractorsForCalendar(m_calendar, m_extractors);
#endif
    }
    if (m_mimeContext) {
        m_repo.extractorsForMessage(m_mimeContext, m_extractors);
    }
    if (!m_text.isEmpty()) {
        m_repo.extractorsForContent(m_text, m_extractors);
    } else if (m_inputType == ExtractorInput::Text && !m_data.isEmpty()) {
        m_text = QString::fromUtf8(m_data);
        m_repo.extractorsForContent(m_text, m_extractors);
    }

    for (const auto &genericResult : m_genericResults) {
        // check if generic extractors identified documents we have custom extractors for
        m_repo.extractorsForJsonLd(genericResult.result, m_extractors);
        // check the unrecognized (vendor-specific) barcodes, if any
        m_repo.extractorsForBarcode(genericResult.barcode.toString(), m_extractors);
    }
}

void ExtractorEnginePrivate::extractCustom()
{
    for (const auto &extractor : m_extractors) {
        switch (extractor.type()) {
            case ExtractorInput::Text:
                // running text extractors on PDF or HTML docs is possible,
                // but only extract the text when really needed
                if (m_text.isEmpty() && m_pdfDoc) {
                    m_text = m_pdfDoc->text();
                }
                if (m_text.isEmpty() && m_htmlDoc) {
                    m_text = m_htmlDoc->root().recursiveContent();
                }
                if (m_text.isEmpty() && !m_data.isEmpty()) {
                    m_text = QString::fromUtf8(m_data);
                }

                if (!m_text.isEmpty()) {
                    executeScript(extractor);
                }
                break;
            case ExtractorInput::Html:
                if (m_htmlDoc) {
                    executeScript(extractor);
                }
                break;
            case ExtractorInput::Pdf:
                if (m_pdfDoc) {
                    executeScript(extractor);
                }
                break;
            case ExtractorInput::PkPass:
                if (m_pass) {
                    executeScript(extractor);
                }
                break;
            case ExtractorInput::ICal:
#ifdef HAVE_KCAL
                if (m_calendar) {
                    executeScript(extractor);
                }
#endif
                break;
            default:
                qCWarning(Log) << "Unexpected extractor type:" << extractor.type();
                break;
        }

        if (!m_result.isEmpty()) {
            m_usedExtractor = extractor.name();
        }
    }
}

void ExtractorEnginePrivate::extractCustomForGenericResults()
{
    for (const auto &genericResult : m_genericResults) {
        // expose genericResult content to custom extractors via Context object
        m_context->m_barcode = genericResult.barcode;
        if (!genericResult.result.empty()) {
            m_context->m_data = m_engine.toScriptValue(genericResult.result);
        }
        m_context->m_pdfPageNum = genericResult.pageNum;

        // check if generic extractors identified documents we have custom extractors for
        const auto prevResults = m_result.size();
        extractCustom();
        m_context->reset();

        // if this didn't find something, take the generic extractor result as-is
        if (prevResults == m_result.size()) {
            std::copy(genericResult.result.begin(), genericResult.result.end(), std::back_inserter(m_result));
        }
    }
}

static void printScriptError(const QJSValue &result)
{
    // don't change the formatting without adjusting KItinerary Workbench too!
    qCWarning(Log).noquote().nospace() << "JS ERROR: [" << result.property(QStringLiteral("fileName")).toString()
        << "]:" << result.property(QStringLiteral("lineNumber")).toInt() << ": " << result.toString();
}

void ExtractorEnginePrivate::executeScript(const Extractor &extractor)
{
    if (extractor.scriptFileName().isEmpty()) {
        return;
    }

    QFile f(extractor.scriptFileName());
    if (!f.open(QFile::ReadOnly)) {
        qCWarning(Log) << "Failed to open extractor script" << f.fileName() << f.errorString();
        return;
    }

    auto result = m_engine.evaluate(QString::fromUtf8(f.readAll()), f.fileName());
    if (result.isError()) {
        printScriptError(result);
        return;
    }

    auto mainFunc = m_engine.globalObject().property(extractor.scriptFunction());
    if (!mainFunc.isCallable()) {
        qCWarning(Log) << "Script entry point not found!" << extractor.scriptFunction();
        return;
    }

    qCDebug(Log) << "Running custom extractor" << extractor.scriptFileName() << extractor.scriptFunction();
    QJSValueList args;
    switch (extractor.type()) {
        case ExtractorInput::Text:
            args = {m_text};
            break;
        case ExtractorInput::Html:
            args = {m_engine.toScriptValue<QObject*>(m_htmlDoc.get())};
            break;
        case ExtractorInput::Pdf:
            args = {m_engine.toScriptValue<QObject*>(m_pdfDoc.get())};
            break;
        case ExtractorInput::PkPass:
            args = {m_engine.toScriptValue<QObject*>(m_pass.get())};
            break;
        case ExtractorInput::ICal:
        {
#ifdef HAVE_KCAL
            const auto events = m_calendar->events();
            for (const auto &event : events) {
                processScriptResult(mainFunc.call({m_engine.toScriptValue(*event.data())}));
            }
#endif
            break;
        }
        default:
            qCWarning(Log) << "Unexpected extractor input type:" << extractor.type();
            break;
    }

    if (!args.isEmpty()) {
        processScriptResult(mainFunc.call(args));
    }
}

void ExtractorEnginePrivate::processScriptResult(const QJSValue &result)
{
    if (result.isError()) {
        printScriptError(result);
        return;
    }

    if (result.isArray()) {
        QJSValueIterator it(result);
        while (it.hasNext()) {
            it.next();
            if (it.value().isObject()) {
                m_result.push_back(QJsonValue::fromVariant(it.value().toVariant()));
            }
        }
    } else if (result.isObject()) {
        m_result.push_back(QJsonValue::fromVariant(result.toVariant()));
    } else {
        qCWarning(Log) << "Invalid result type from script";
    }
}

void ExtractorEngine::setUseSeparateProcess(bool separateProcess)
{
    if (!separateProcess) {
        d->m_externalExtractor.clear();
        return;
    }

    // find external extractor
    QFileInfo fi(QLatin1String(CMAKE_INSTALL_FULL_LIBEXECDIR_KF5) + QLatin1String("/kitinerary-extractor"));
    if (!fi.exists() && !fi.isFile() && !fi.isExecutable()) {
        qCCritical(Log) << "Cannot find external extractor:" << fi.fileName();
        return;
    }
    d->m_externalExtractor = fi.canonicalFilePath();
}

void ExtractorEngine::setAdditionalExtractors(std::vector<Extractor> &&extractors)
{
    d->m_additionalExtractors = std::move(extractors);
}

QString ExtractorEngine::usedCustomExtractor() const
{
    return d->m_usedExtractor;
}
