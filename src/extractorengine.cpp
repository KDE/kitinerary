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
#include "logging.h"
#include "pdfdocument.h"

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

using namespace KItinerary;

namespace KItinerary {

class ContextObject;

class ExtractorEnginePrivate {
public:
    void setupEngine();
    void executeScript();
    void extractPass();

    const Extractor *m_extractor = nullptr;
    ContextObject *m_context = nullptr;
    QString m_text;
    PdfDocument *m_pdfDoc = nullptr;
    KPkPass::BoardingPass *m_pass;
    QJsonArray m_result;
    QJSEngine m_engine;
};

class JsonLdJsApi : public QObject
{
    Q_OBJECT
public:
    explicit JsonLdJsApi(QJSEngine *engine)
        : QObject(engine)
        , m_engine(engine)
    {
    }

    Q_INVOKABLE QJSValue newObject(const QString &typeName) const;
    Q_INVOKABLE QDateTime toDateTime(const QString &dtStr, const QString &format, const QString &localeName) const;

private:
    QJSEngine *m_engine;
};

QJSValue JsonLdJsApi::newObject(const QString &typeName) const
{
    auto v = m_engine->newObject();
    v.setProperty(QStringLiteral("@type"), typeName);
    return v;
}

QDateTime JsonLdJsApi::toDateTime(const QString &dtStr, const QString &format, const QString &localeName) const
{
    QLocale locale(localeName);
    const auto dt = locale.toDateTime(dtStr, format);
    if (dt.isValid()) {
        return dt;
    }

    // try harder for the "MMM" month format
    // QLocale expects the exact string in QLocale::shortMonthName(), while we often encounter a three
    // letter month identifier. For en_US that's the same, for Swedish it isn't though for example. So
    // let's try to fix up the month identifiers to the full short name.
    if (format.contains(QLatin1String("MMM"))) {
        auto dtStrFixed = dtStr;
        for (int i = 0; i < 12; ++i) {
            const auto monthName = locale.monthName(i, QLocale::ShortFormat);
            dtStrFixed = dtStrFixed.replace(monthName.left(3), monthName);
        }
        return locale.toDateTime(dtStrFixed, format);
    }
    return dt;
}

class ContextObject : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QDateTime senderDate MEMBER m_senderDate)

public:
    QDateTime m_senderDate;
};
}

void ExtractorEnginePrivate::setupEngine()
{
    m_context = new ContextObject; // will be deleted by QJSEngine taking ownership
    m_engine.installExtensions(QJSEngine::ConsoleExtension);
    auto jsApi = new JsonLdJsApi(&m_engine);
    m_engine.globalObject().setProperty(QStringLiteral("JsonLd"), m_engine.newQObject(jsApi));
    m_engine.globalObject().setProperty(QStringLiteral("Context"), m_engine.newQObject(m_context));
}


ExtractorEngine::ExtractorEngine()
    : d(new ExtractorEnginePrivate)
{
    d->setupEngine();
}

ExtractorEngine::ExtractorEngine(ExtractorEngine &&) = default;
ExtractorEngine::~ExtractorEngine() = default;

void ExtractorEngine::clear()
{
    d->m_text.clear();
    d->m_pdfDoc = nullptr;
    d->m_pass = nullptr;
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
    d->m_pass = qobject_cast<KPkPass::BoardingPass*>(pass);
}

void ExtractorEngine::setSenderDate(const QDateTime &dt)
{
    d->m_context->m_senderDate = dt;
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

    auto mainFunc = m_engine.globalObject().property(QLatin1String("main"));
    if (!mainFunc.isCallable()) {
        qCWarning(Log) << "Script has no main() function!";
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
        m_result = QJsonArray::fromVariantList(result.toVariant().toList());
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
        switch (m_pass->transitType()) {
            case KPkPass::BoardingPass::Air:
                res.insert(QLatin1String("@type"), QLatin1String("FlightReservation"));
                resFor.insert(QLatin1String("@type"), QLatin1String("Flight"));
                break;
            // TODO expand once we have test files for train tickets
            default:
                return;
        }
        res.insert(QLatin1String("reservationFor"), resFor);
        m_result.push_back(res);
    }

    // extract structured data from a pkpass, if the extractor script hasn't done so already
    auto res = m_result.at(0).toObject();
    auto resFor = res.value(QLatin1String("reservationFor")).toObject();

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

#include "extractorengine.moc"
