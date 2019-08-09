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
#include "genericpdfextractor_p.h"
#include "genericpkpassextractor_p.h"
#include "genericuic918extractor_p.h"
#include "htmldocument.h"
#include "iatabcbpparser.h"
#include "jsonlddocument.h"
#include "logging.h"
#include "pdf/pdfdocument.h"
#include "structureddataextractor_p.h"
#include "uic9183parser.h"

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
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLocale>
#include <QJSEngine>
#include <QJSValueIterator>

#include <cstring>

using namespace KItinerary;

namespace KItinerary {

class ExtractorEnginePrivate {
public:
    void setupEngine();
    void resetContent();

    void setContent(KMime::Content *content);
    void setContext(KMime::Content *context);
    void setContextDate(const QDateTime &dt);

    void extractRecursive(KMime::Content *content);
    void extractDocument();
    void extractStructured();
    void extractCustom();
    void extractGeneric();

    void executeScript(const Extractor *extractor);
    void processScriptResult(const QJSValue &result);

    std::vector<const Extractor*> m_extractors;
    JsApi::Barcode *m_barcodeApi = nullptr;
    JsApi::Context *m_context = nullptr;
    JsApi::JsonLd *m_jsonLdApi = nullptr;
    QString m_text;
    QByteArray m_data;
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
    QJsonArray m_result;
    QJSEngine m_engine;
    ExtractorRepository m_repo;
    BarcodeDecoder m_barcodeDecoder;
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

ExtractorEngine::ExtractorEngine()
    : d(new ExtractorEnginePrivate)
{
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
}

void ExtractorEnginePrivate::resetContent()
{
    m_text.clear();
    m_data.clear();
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

void ExtractorEnginePrivate::setContent(KMime::Content *content)
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
        m_pass = make_owning_ptr(KPkPass::Pass::fromData(content->decodedContent()));
    } else if (mtType == ExtractorInput::ICal || fnType == ExtractorInput::ICal) {
#ifdef HAVE_KCAL
        m_calendar.reset(new KCalendarCore::MemoryCalendar(QTimeZone()));
        KCalendarCore::ICalFormat format;
        if (format.fromRawString(m_calendar, content->decodedContent())) {
            m_calendar->setProductId(format.loadedProductId());
        } else {
            m_calendar.reset();
        }
#endif
    } else if (mtType == ExtractorInput::Pdf || fnType == ExtractorInput::Pdf) {
        m_pdfDoc = make_owning_ptr(PdfDocument::fromData(content->decodedContent()));
    } else if (mtType == ExtractorInput::Html) {
        m_htmlDoc = make_owning_ptr(HtmlDocument::fromData(content->decodedContent()));
    } else if ( (mtType == ExtractorInput::Text) || (!ct && content->isTopLevel())) {
        m_text = content->decodedText();
    }

    m_mimeContent = (ct && ct->isMultipart()) ? content : nullptr;
}

void ExtractorEngine::setData(const QByteArray &data, const QString &fileName)
{
    // let's not even try to parse anything with implausible size
    if (data.size() <= 4 || data.size() > 4000000) {
        return;
    }

    const auto nameType = ExtractorInput::typeFromFileName(fileName);
    const auto contentType = ExtractorInput::typeFromContent(data);
    qWarning() << nameType << contentType << fileName;
    setData(data, nameType == ExtractorInput::Unknown ? contentType : nameType);
}

void ExtractorEngine::setData(const QByteArray &data, ExtractorInput::Type type)
{
    // let's not even try to parse anything with implausible size
    if (data.size() <= 4 || data.size() > 4000000) {
        return;
    }

    switch (type) {
        case ExtractorInput::PkPass:
            d->m_pass = make_owning_ptr(KPkPass::Pass::fromData(data));
            break;
        case ExtractorInput::Pdf:
            d->m_pdfDoc = make_owning_ptr(PdfDocument::fromData(data));
            break;
        case ExtractorInput::Html:
            d->m_htmlDoc = make_owning_ptr(HtmlDocument::fromData(data));
            break;
        case ExtractorInput::ICal:
        {
#ifdef HAVE_KCAL
            d->m_calendar.reset(new KCalendarCore::MemoryCalendar(QTimeZone()));
            KCalendarCore::ICalFormat format;
            if (format.fromRawString(d->m_calendar, data)) {
                d->m_calendar->setProductId(format.loadedProductId());
                break;
            }
            qCDebug(Log) << "Failed to parse iCal content.";
            d->m_calendar.reset();
#else
            qCDebug(Log) << "Trying to exctract ical file, but ical support is not enabled.";
#endif
            break;
        }
        case ExtractorInput::Text:
            d->m_text = QString::fromUtf8(data);
            break;
        case ExtractorInput::Email:
            d->m_ownedMimeContent.reset(new KMime::Message);
            d->m_ownedMimeContent->setContent(KMime::CRLFtoLF(data));
            d->m_ownedMimeContent->parse();
            setContent(d->m_ownedMimeContent.get());
            break;
        case ExtractorInput::JsonLd:
        {
            // pass through JSON data, so the using code can apply post-processing to that
            const auto doc = QJsonDocument::fromJson(data);
            if (doc.isObject()) {
                d->m_result.push_back(doc.object());
            } else if (doc.isArray()) {
                d->m_result = doc.array();
            }
            break;
        }
        default:
            d->m_data = data;
            break;
    }
}

void ExtractorEnginePrivate::setContext(KMime::Content *context)
{
    m_mimeContext = context;
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

void ExtractorEnginePrivate::setContextDate(const QDateTime &dt)
{
    m_context->m_senderDate = dt;
    m_jsonLdApi->setContextDate(dt);
    m_barcodeApi->setContextDate(dt);
    m_genericPdfExtractor.setContextDate(dt);
}

void ExtractorEngine::setContent(KMime::Content *content)
{
    d->setContent(content);
}

void ExtractorEngine::setContext(KMime::Content *context)
{
    d->setContext(context);
}

void ExtractorEngine::setContextDate(const QDateTime &dt)
{
    d->setContextDate(dt);
}

QJsonArray ExtractorEngine::extract()
{
    if (d->m_mimeContent) {
        d->extractRecursive(d->m_mimeContent);
    } else {
        d->extractDocument();
    }

    return d->m_result;
}

void ExtractorEnginePrivate::extractRecursive(KMime::Content *content)
{
    QJsonArray aggregatedResult;
    const auto children = content->contents();
    for (const auto child : children) {
        resetContent();
        setContent(child);
        if (m_mimeContent) {
            extractRecursive(m_mimeContent);
        } else {
            extractDocument();
        }

        // the extractor takes early exits if data has been found, so make it look like that isn't the case
        std::copy(m_result.begin(), m_result.end(), std::back_inserter(aggregatedResult));
        m_result = {};
    }

    m_result = std::move(aggregatedResult);
}

void ExtractorEnginePrivate::extractDocument()
{
    // structured content
    extractStructured();
    if (!m_result.isEmpty()) {
        return;
    }

    // custom extractors
    m_extractors.clear();
    if (m_pass) {
        m_extractors = m_repo.extractorsForPass(m_pass.get());
#ifdef HAVE_KCAL
    } else if (m_calendar) {
        m_extractors = m_repo.extractorsForCalendar(m_calendar);
#endif
    }
    if (m_extractors.empty() && m_mimeContext) {
        m_extractors = m_repo.extractorsForMessage(m_mimeContext);
    }
    extractCustom();

    // generic extractors
    extractGeneric();
}

void ExtractorEnginePrivate::extractStructured()
{
    if (m_htmlDoc) {
        qCDebug(Log) << "Looking for structured annotations...";
        const auto res = StructuredDataExtractor::extract(m_htmlDoc.get());
        std::copy(res.begin(), res.end(), std::back_inserter(m_result));
    }
}

void ExtractorEnginePrivate::extractCustom()
{
    for (const auto extractor : m_extractors) {
        switch (extractor->type()) {
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
                qCWarning(Log) << "Unexpected extractor type:" << extractor->type();
                break;
        }

        if (!m_result.isEmpty()) {
            break;
        }
    }
}

void ExtractorEnginePrivate::extractGeneric()
{
    if (m_pass) {
        if (m_result.size() > 1) { // a pkpass file contains exactly one boarding pass
            return;
        }
        if (m_result.isEmpty()) {
            m_result.push_back(QJsonObject());
        }

        auto res = m_result.at(0).toObject();
        res = GenericPkPassExtractor::extract(m_pass.get(), res, m_context->m_senderDate);
        m_result[0] = res;
    } else if (m_pdfDoc && m_result.isEmpty()) {
        QJsonArray genericResult;
        m_genericPdfExtractor.extract(m_pdfDoc.get(), genericResult);

        // check if generic extractors identified documents we have custom extractors for
        m_extractors = m_repo.extractorsForJsonLd(genericResult);
        extractCustom();

        // check the unrecognized (vendor-specific) barcodes, if any
        const auto unrecognizedCodes = m_genericPdfExtractor.unrecognizedBarcodes();
        for (const auto &code : unrecognizedCodes) {
            m_extractors = m_repo.extractorsForBarcode(code);
            extractCustom();
        }

        // if none of that found something, take the generic extractor result as-is
        if (m_result.isEmpty()) {
            m_result = genericResult;
        }
    } else if (!m_text.isEmpty() && m_result.isEmpty()) {
        if (IataBcbpParser::maybeIataBcbp(m_text)) {
            const auto res = IataBcbpParser::parse(m_text, m_context->m_senderDate.date());
            m_result = JsonLdDocument::toJson(res);
        }
    } else if (!m_data.isEmpty() && m_result.isEmpty()) {
        if (Uic9183Parser::maybeUic9183(m_data)) {
            GenericUic918Extractor::extract(m_data, m_result, m_context->m_senderDate);
            return;
        }
        // try again as text
        m_text = QString::fromUtf8(m_data);
        extractGeneric();
    }
}

void ExtractorEnginePrivate::executeScript(const Extractor *extractor)
{
    Q_ASSERT(extractor);
    if (extractor->scriptFileName().isEmpty()) {
        return;
    }

    QFile f(extractor->scriptFileName());
    if (!f.open(QFile::ReadOnly)) {
        qCWarning(Log) << "Failed to open extractor script" << f.fileName() << f.errorString();
        return;
    }

    auto result = m_engine.evaluate(QString::fromUtf8(f.readAll()), f.fileName());
    if (result.isError()) {
        qCWarning(Log) << "Script parsing error in" << result.property(QStringLiteral("fileName")).toString()
                                << ':' << result.property(QStringLiteral("lineNumber")).toInt() << result.toString();
        return;
    }

    auto mainFunc = m_engine.globalObject().property(extractor->scriptFunction());
    if (!mainFunc.isCallable()) {
        qCWarning(Log) << "Script entry point not found!" << extractor->scriptFunction();
        return;
    }

    qCDebug(Log) << "Running custom extractor" << extractor->scriptFileName() << extractor->scriptFunction();
    QJSValueList args;
    switch (extractor->type()) {
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
            qCWarning(Log) << "Unexpected extractor input type:" << extractor->type();
            break;
    }

    if (!args.isEmpty()) {
        processScriptResult(mainFunc.call(args));
    }
}

void ExtractorEnginePrivate::processScriptResult(const QJSValue &result)
{
    if (result.isError()) {
        qCWarning(Log) << "Script execution error in" << result.property(QStringLiteral("fileName")).toString()
                                << ':' << result.property(QStringLiteral("lineNumber")).toInt() << result.toString();
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
