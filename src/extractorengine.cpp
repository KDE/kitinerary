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

#include "extractorengine.h"
#include "extractor.h"
#include "jsonlddocument.h"
#include "logging.h"
#include "pdfdocument.h"

#include "jsapi/barcode.h"
#include "jsapi/context.h"
#include "jsapi/jsonld.h"

#include <KPkPass/Barcode>
#include <KPkPass/BoardingPass>
#include <KPkPass/Location>

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
    void executeScript();
    void extractPass();
    void extractBoardingPass(QJsonObject &resFor);
    void extractEventTicketPass(QJsonObject &resFor);

    const Extractor *m_extractor = nullptr;
    JsApi::Barcode *m_barcodeApi = nullptr;
    JsApi::Context *m_context = nullptr;
    JsApi::JsonLd *m_jsonLdApi = nullptr;
    QString m_text;
    PdfDocument *m_pdfDoc = nullptr;
    KPkPass::Pass *m_pass;
    QJsonArray m_result;
    QJSEngine m_engine;
};

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
    d->m_text.clear();
    d->m_pdfDoc = nullptr;
    d->m_pass = nullptr;
    d->m_result = {};
    d->m_context->m_senderDate = {};
}

void ExtractorEngine::setExtractor(const Extractor *extractor)
{
    d->m_extractor = extractor;
}

void ExtractorEngine::setText(const QString &text)
{
    d->m_text = text;
}

void ExtractorEngine::setPdfDocument(PdfDocument *pdfDoc)
{
    d->m_pdfDoc = pdfDoc;
    if (pdfDoc) {
        d->m_text = pdfDoc->text();
    }
}

void ExtractorEngine::setPass(KPkPass::Pass *pass)
{
    d->m_pass = pass;
}

void ExtractorEngine::setSenderDate(const QDateTime &dt)
{
    d->m_context->m_senderDate = dt;
    d->m_jsonLdApi->setContextDate(dt);
    d->m_barcodeApi->setContextDate(dt.date());
}

QJsonArray ExtractorEngine::extract()
{
    if (!d->m_extractor) {
        return {};
    }
    switch (d->m_extractor->type()) {
        case Extractor::Text:
            if (d->m_text.isEmpty()) {
                return {};
            }
            d->executeScript();
            break;
        case Extractor::Pdf:
            if (!d->m_pdfDoc) {
                return {};
            }
            d->executeScript();
            break;
        case Extractor::PkPass:
            if (!d->m_pass) {
                return {};
            }
            d->executeScript();
            d->extractPass();
            break;
    }

    return d->m_result;
}

void ExtractorEnginePrivate::executeScript()
{
    Q_ASSERT(m_extractor);
    if (m_extractor->scriptFileName().isEmpty()) {
        return;
    }

    QFile f(m_extractor->scriptFileName());
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

    auto mainFunc = m_engine.globalObject().property(m_extractor->scriptFunction());
    if (!mainFunc.isCallable()) {
        qCWarning(Log) << "Script entry point not found!" << m_extractor->scriptFunction();
        return;
    }

    QJSValueList args;
    switch (m_extractor->type()) {
        case Extractor::Text:
            args = {m_text};
            break;
        case Extractor::Pdf:
            args = {m_engine.toScriptValue<QObject*>(m_pdfDoc)};
            break;
        case Extractor::PkPass:
            args = {m_engine.toScriptValue<QObject*>(m_pass)};
            break;
    }

    result = mainFunc.call(args);
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
        m_result = { QJsonValue::fromVariant(result.toVariant()) };
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
        if (auto boardingPass = qobject_cast<KPkPass::BoardingPass*>(m_pass)) {
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
