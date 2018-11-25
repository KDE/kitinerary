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
#include "extractorengine.h"
#include "extractor.h"
#include "extractorrepository.h"
#include "genericpdfextractor.h"
#include "htmldocument.h"
#include "jsonlddocument.h"
#include "logging.h"
#include "pdfdocument.h"
#include "structureddataextractor.h"

#include "jsapi/barcode.h"
#include "jsapi/context.h"
#include "jsapi/jsonld.h"

#ifdef HAVE_KCAL
#include <KCalCore/MemoryCalendar>
#include <KCalCore/Event>
#include <KCalCore/ICalFormat>
#endif

#include <KPkPass/Barcode>
#include <KPkPass/BoardingPass>
#include <KPkPass/Location>

#include <KMime/Content>

#include <QDateTime>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLocale>
#include <QJSEngine>
#include <QJSValueIterator>

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
    void extractPass();
    void extractBoardingPass(QJsonObject &resFor);
    void extractEventTicketPass(QJsonObject &resFor);

    std::vector<const Extractor*> m_extractors;
    JsApi::Barcode *m_barcodeApi = nullptr;
    JsApi::Context *m_context = nullptr;
    JsApi::JsonLd *m_jsonLdApi = nullptr;
    QString m_text;
    std::unique_ptr<HtmlDocument, std::function<void(HtmlDocument*)>> m_htmlDoc;
    std::unique_ptr<PdfDocument, std::function<void(PdfDocument*)>> m_pdfDoc;
    std::unique_ptr<KPkPass::Pass, std::function<void(KPkPass::Pass*)>> m_pass;
#ifdef HAVE_KCAL
    KCalCore::Calendar::Ptr m_calendar;
#endif
    KMime::Content *m_mimeContent = nullptr;
    KMime::Content *m_mimeContext = nullptr;
    GenericPdfExtractor m_genericPdfExtractor;
    QJsonArray m_result;
    QJSEngine m_engine;
    ExtractorRepository m_repo;
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
    m_engine.globalObject().setProperty(QStringLiteral("Barcode"), m_engine.newQObject(m_barcodeApi));
    m_engine.globalObject().setProperty(QStringLiteral("Context"), m_engine.newQObject(m_context));
}


ExtractorEngine::ExtractorEngine()
    : d(new ExtractorEnginePrivate)
{
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
}

void ExtractorEnginePrivate::resetContent()
{
    m_text.clear();
    m_pdfDoc.reset();
    m_htmlDoc.reset();
    m_pass.reset();
#ifdef HAVE_KCAL
    m_calendar.reset();
#endif
    m_mimeContent = nullptr;
}

void ExtractorEngine::setExtractors(std::vector<const Extractor*> &&extractors)
{
    d->m_extractors = extractors;
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

void ExtractorEngine::setCalendar(const QSharedPointer<KCalCore::Calendar> &calendar)
{
#ifdef HAVE_KCAL
    d->m_calendar = calendar;
#else
    Q_UNUSED(calendar);
#endif
}

static bool isContentType(KMime::Content *content, KMime::Headers::ContentType *ct, const char *mimeType, const char *ext)
{
    if (ct && ct->mimeType() == mimeType) {
        return true;
    }
    if (ct && ct->name().endsWith(QLatin1String(ext))) {
        return true;
    }
    const auto cd = content->contentDisposition(false);
    return cd && cd->filename().endsWith(QLatin1String(ext));
}

void ExtractorEnginePrivate::setContent(KMime::Content *content)
{
    setContext(content);

    const auto ct = content->contentType(false);
    if (isContentType(content, ct, "application/vnd.apple.pkpass", ".pkpass")) {
        m_pass = make_owning_ptr(KPkPass::Pass::fromData(content->decodedContent()));
    } else if (isContentType(content, ct, "text/calendar", ".ics")) {
#ifdef HAVE_KCAL
        m_calendar.reset(new KCalCore::MemoryCalendar(QTimeZone()));
        KCalCore::ICalFormat format;
        if (!format.fromRawString(m_calendar, content->decodedContent())) {
            m_calendar.reset();
        }
#endif
    } else if (isContentType(content, ct, "application/pdf", ".pdf")) {
        m_pdfDoc = make_owning_ptr(PdfDocument::fromData(content->decodedContent()));
    } else if (ct && ct->isHTMLText()) {
        m_htmlDoc = make_owning_ptr(HtmlDocument::fromData(content->decodedContent()));
    } else if ( (ct && ct->isPlainText()) || (!ct && content->isTopLevel())) {
        m_text = content->decodedText();
    }

    m_mimeContent = (ct && ct->isMultipart()) ? content : nullptr;
}

void ExtractorEnginePrivate::setContext(KMime::Content *context)
{
    m_mimeContext = context;
    auto dateHdr = context->header<KMime::Headers::Date>();
    while (!dateHdr && context->parent()) {
        context = context->parent();
        dateHdr = context->header<KMime::Headers::Date>();
    }
    if (dateHdr) {
        setContextDate(dateHdr->dateTime());
    }
}

void ExtractorEnginePrivate::setContextDate(const QDateTime &dt)
{
    m_context->m_senderDate = dt;
    m_jsonLdApi->setContextDate(dt);
    m_barcodeApi->setContextDate(dt.date());
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
    for (const auto child : content->contents()) {
        resetContent();
        setContent(child);
        if (m_mimeContent) {
            extractRecursive(m_mimeContent);
        } else {
            extractDocument();
        }
    }
}

void ExtractorEnginePrivate::extractDocument()
{
    // structured content
    extractStructured();
    if (!m_result.isEmpty()) {
        return;
    }

    // custom extractors
    if (m_pass) {
        m_extractors = m_repo.extractorsForPass(m_pass.get());
    } else if (m_mimeContext) {
        m_extractors = m_repo.extractorsForMessage(m_mimeContext);
    }
    extractCustom();

    // generic extractors
    extractGeneric();

    // check if generic extractors identified documents we have custom extractors for
    m_extractors = m_repo.extractorsForJsonLd(m_result);
    extractCustom();

    // check the unrecognized (vendor-specific) barcodes, if any
    if (m_pdfDoc) {
        for (const auto &code : m_genericPdfExtractor.unrecognizedBarcodes()) {
            m_extractors = m_repo.extractorsForBarcode(code);
            qDebug() << code << m_extractors.size();
            extractCustom();
        }
    }
}

void ExtractorEnginePrivate::extractStructured()
{
    if (m_htmlDoc) {
        qCDebug(Log) << "Looking for structured annotations...";
        for (const auto &v : StructuredDataExtractor::extract(m_htmlDoc.get())) {
            m_result.push_back(v);
        }
    }
}

void ExtractorEnginePrivate::extractCustom()
{
    for (const auto extractor : m_extractors) {
        switch (extractor->type()) {
            case Extractor::Text:
                // running text extractors on PDF or HTML docs is possible,
                // but only extract the text when really needed
                if (m_text.isEmpty() && m_pdfDoc) {
                    m_text = m_pdfDoc->text();
                }
                if (m_text.isEmpty() && m_htmlDoc) {
                    m_text = m_htmlDoc->root().recursiveContent();
                }

                if (!m_text.isEmpty()) {
                    executeScript(extractor);
                }
                break;
            case Extractor::Html:
                if (m_htmlDoc) {
                    executeScript(extractor);
                }
                break;
            case Extractor::Pdf:
                if (m_pdfDoc) {
                    executeScript(extractor);
                }
                break;
            case Extractor::PkPass:
                if (m_pass) {
                    executeScript(extractor);
                }
                break;
            case Extractor::ICal:
#ifdef HAVE_KCAL
                if (m_calendar) {
                    executeScript(extractor);
                }
#endif
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
        extractPass();
    } else if (m_pdfDoc && m_result.isEmpty()) {
        m_genericPdfExtractor.extract(m_pdfDoc.get(), m_result);
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
        qCWarning(Log) << "Script parsing error in" << result.property(QLatin1String("fileName")).toString()
                                << ':' << result.property(QLatin1String("lineNumber")).toInt() << result.toString();
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
        case Extractor::Text:
            args = {m_text};
            break;
        case Extractor::Html:
            args = {m_engine.toScriptValue<QObject*>(m_htmlDoc.get())};
            break;
        case Extractor::Pdf:
            args = {m_engine.toScriptValue<QObject*>(m_pdfDoc.get())};
            break;
        case Extractor::PkPass:
            args = {m_engine.toScriptValue<QObject*>(m_pass.get())};
            break;
        case Extractor::ICal:
#ifdef HAVE_KCAL
            for (const auto &event : m_calendar->events()) {
                processScriptResult(mainFunc.call({m_engine.toScriptValue(*event.data())}));
            }
#endif
            break;
    }

    if (!args.isEmpty()) {
        processScriptResult(mainFunc.call(args));
    }
}

void ExtractorEnginePrivate::processScriptResult(const QJSValue &result)
{
    if (result.isError()) {
        qCWarning(Log) << "Script execution error in" << result.property(QLatin1String("fileName")).toString()
                                << ':' << result.property(QLatin1String("lineNumber")).toInt() << result.toString();
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

void ExtractorEnginePrivate::extractPass()
{
    if (m_result.size() > 1) { // a pkpass file contains exactly one boarding pass
        return;
    }

    if (m_result.isEmpty()) { // no script run, so we need to create the top-level element ourselves
        QJsonObject res;
        QJsonObject resFor;
        if (auto boardingPass = qobject_cast<KPkPass::BoardingPass*>(m_pass.get())) {
            switch (boardingPass->transitType()) {
                case KPkPass::BoardingPass::Air:
                    res.insert(QLatin1String("@type"), QLatin1String("FlightReservation"));
                    resFor.insert(QLatin1String("@type"), QLatin1String("Flight"));
                    break;
                // TODO expand once we have test files for train tickets
                default:
                    return;
            }
        } else {
            switch (m_pass->type()) {
                case KPkPass::Pass::EventTicket:
                    res.insert(QLatin1String("@type"), QLatin1String("EventReservation"));
                    resFor.insert(QLatin1String("@type"), QLatin1String("Event"));
                    break;
                default:
                    return;
            }
        }
        res.insert(QLatin1String("reservationFor"), resFor);
        m_result.push_back(res);
    }

    // extract structured data from a pkpass, if the extractor script hasn't done so already
    auto res = m_result.at(0).toObject();
    auto resFor = res.value(QLatin1String("reservationFor")).toObject();
    switch (m_pass->type()) {
        case KPkPass::Pass::BoardingPass:
            extractBoardingPass(resFor);
            break;
        case KPkPass::Pass::EventTicket:
            extractEventTicketPass(resFor);
            break;
        default:
            return;
    }

    // barcode contains the ticket token
    if (!m_pass->barcodes().isEmpty() && !res.contains(QLatin1String("reservedTicket"))) {
        const auto barcode = m_pass->barcodes().at(0);
        QString token;
        switch (barcode.format()) {
            case KPkPass::Barcode::QR:
                token += QLatin1String("qrCode:");
                break;
            case KPkPass::Barcode::Aztec:
                token += QLatin1String("aztecCode:");
                break;
            default:
                break;
        }
        token += barcode.message();
        QJsonObject ticket;
        ticket.insert(QLatin1String("@type"), QLatin1String("Ticket"));
        ticket.insert(QLatin1String("ticketToken"), token);
        res.insert(QLatin1String("reservedTicket"), ticket);
    }

    res.insert(QLatin1String("reservationFor"), resFor);

    // associate the pass with the result, so we can find the pass again for display
    if (!m_pass->passTypeIdentifier().isEmpty() && !m_pass->serialNumber().isEmpty()) {
        res.insert(QLatin1String("pkpassPassTypeIdentifier"), m_pass->passTypeIdentifier());
        res.insert(QLatin1String("pkpassSerialNumber"), m_pass->serialNumber());
    }

    m_result[0] = res;
}

void ExtractorEnginePrivate::extractBoardingPass(QJsonObject &resFor)
{
    // "relevantDate" is the best guess for the boarding time
    if (m_pass->relevantDate().isValid() && !resFor.contains(QLatin1String("boardingTime"))) {
        resFor.insert(QLatin1String("boardingTime"), m_pass->relevantDate().toString(Qt::ISODate));
    }
    // look for common field names containing the boarding time, if we still have no idea
    if (!resFor.contains(QLatin1String("boardingTime"))) {
        for (const auto &field : m_pass->fields()) {
            if (!field.key().contains(QLatin1String("boarding"), Qt::CaseInsensitive)) {
                continue;
            }
            const auto time = QTime::fromString(field.value().toString());
            if (time.isValid()) {
                // this misses date, but the postprocessor will fill that in
                resFor.insert(QLatin1String("boardingTime"), QDateTime(QDate(1, 1, 1), time).toString(Qt::ISODate));
                break;
            }
        }
    }

    // location is the best guess for the departure airport geo coordinates
    auto depAirport = resFor.value(QLatin1String("departureAirport")).toObject();
    if (depAirport.isEmpty()) {
        depAirport.insert(QLatin1String("@type"), QLatin1String("Airport"));
    }
    auto depGeo = depAirport.value(QLatin1String("geo")).toObject();
    if (m_pass->locations().size() == 1 && depGeo.isEmpty()) {
        const auto loc = m_pass->locations().at(0);
        depGeo.insert(QLatin1String("@type"), QLatin1String("GeoCoordinates"));
        depGeo.insert(QLatin1String("latitude"), loc.latitude());
        depGeo.insert(QLatin1String("longitude"), loc.longitude());
        depAirport.insert(QLatin1String("geo"), depGeo);
        resFor.insert(QLatin1String("departureAirport"), depAirport);
    }

    // organizationName is the best guess for airline name
    auto airline = resFor.value(QLatin1String("airline")).toObject();
    if (airline.isEmpty()) {
        airline.insert(QLatin1String("@type"), QLatin1String("Airline"));
    }
    if (!airline.contains(QLatin1String("name"))) {
        airline.insert(QLatin1String("name"), m_pass->organizationName());
    }
    resFor.insert(QLatin1String("airline"), airline);
}

void ExtractorEnginePrivate::extractEventTicketPass(QJsonObject &resFor)
{
    if (!resFor.contains(QLatin1String("name"))) {
        resFor.insert(QLatin1String("name"), m_pass->description());
    }

    // "relevantDate" is the best guess for the start time
    if (m_pass->relevantDate().isValid() && !resFor.contains(QLatin1String("startDate"))) {
        resFor.insert(QLatin1String("startDate"), m_pass->relevantDate().toString(Qt::ISODate));
    }

    // location is the best guess for the venue
    auto venue = resFor.value(QLatin1String("location")).toObject();
    if (venue.isEmpty()) {
        venue.insert(QLatin1String("@type"), QLatin1String("Place"));
    }
    auto geo = venue.value(QLatin1String("geo")).toObject();
    if (!m_pass->locations().isEmpty() && geo.isEmpty()) {
        const auto loc = m_pass->locations().at(0);
        geo.insert(QLatin1String("@type"), QLatin1String("GeoCoordinates"));
        geo.insert(QLatin1String("latitude"), loc.latitude());
        geo.insert(QLatin1String("longitude"), loc.longitude());
        venue.insert(QLatin1String("geo"), geo);
        venue.insert(QLatin1String("name"), loc.relevantText());
        resFor.insert(QLatin1String("location"), venue);
    }
}
