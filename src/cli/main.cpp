/*
    Copyright (C) 2019 Volker Krause <vkrause@kde.org>

    This program is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This program is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include <config-kitinerary.h>
#include <kitinerary_version.h>

#include <KItinerary/ExtractorEngine>
#include <KItinerary/ExtractorPostprocessor>
#include <KItinerary/HtmlDocument>
#include <KItinerary/JsonLdDocument>
#include <KItinerary/PdfDocument>

#include <KPkPass/Pass>

#include <KMime/Message>

#include <KCalCore/MemoryCalendar>
#include <KCalCore/ICalFormat>

#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDebug>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QObject>

#include <iostream>

using namespace KItinerary;

static void printCapabilities()
{
    std::cout << "HTML support        : "
#ifdef HAVE_LIBXML2
              << "libxml2"
#else
              << "not available"
#endif
              << std::endl;

    std::cout << "PDF support         : "
#ifdef HAVE_POPPLER
              << "poppler"
#else
              << "not available"
#endif
              << std::endl;

    std::cout << "iCal support        : "
#ifdef HAVE_KCAL
              << "kcal"
#else
              << "not available"
#endif
              << std::endl;

    std::cout << "Barcode decoder     : "
#ifdef HAVE_ZXING
              << "zxing"
#elif defined(HAVE_ZXING_OLD)
              << "legacy zxing"
#else
              << "not available"
#endif
              << std::endl;

    std::cout << "Phone number decoder: "
#ifdef HAVE_PHONENUMBER
              << "libphonenumber"
#else
              << "not available"
#endif
              << std::endl;
}

int main(int argc, char** argv)
{
    QCoreApplication::setApplicationName(QStringLiteral("kitinerary-extractor"));
    QCoreApplication::setApplicationVersion(QStringLiteral(KITINERARY_VERSION_STRING));
    QCoreApplication::setOrganizationDomain(QStringLiteral("kde.org"));
    QCoreApplication::setOrganizationName(QStringLiteral("KDE"));
    QCoreApplication app(argc, argv);

    QCommandLineParser parser;
    parser.setApplicationDescription(QStringLiteral("Command line itinerary extractor."));
    parser.addHelpOption();
    parser.addVersionOption();
    QCommandLineOption capOpt({QStringLiteral("capabilities")}, QStringLiteral("Show available extraction capabilities."));
    parser.addOption(capOpt);
    QCommandLineOption ctxOpt({QStringLiteral("c"), QStringLiteral("context-date")}, QStringLiteral("ISO date/time for when this data has been received."), QStringLiteral("date"));
    parser.addOption(ctxOpt);
    QCommandLineOption typeOpt({QStringLiteral("t"), QStringLiteral("type")}, QStringLiteral("Type of the input data [mime, pdf, pkpass, ical, html]."), QStringLiteral("type"));
    parser.addOption(typeOpt);

    parser.addPositionalArgument(QStringLiteral("input"), QStringLiteral("File to extract data from, omit for using stdin."));
    parser.process(app);

    if (parser.isSet(capOpt)) {
        printCapabilities();
        return 0;
    }

    QFile f;
    if (parser.positionalArguments().size() == 1) {
        f.setFileName(parser.positionalArguments().at(0));
        if (!f.open(QFile::ReadOnly)) {
            std::cerr << qPrintable(f.errorString()) << std::endl;
            return 1;
        }
    } else {
        f.open(stdin, QFile::ReadOnly);
    }

    auto contextDt = QDateTime::fromString(parser.value(ctxOpt), Qt::ISODate);
    if (!contextDt.isValid()) {
        contextDt = QDateTime::currentDateTime();
    }

    ExtractorEngine engine;
    engine.setContextDate(contextDt);

    std::unique_ptr<KPkPass::Pass> pass;
    std::unique_ptr<HtmlDocument> htmlDoc;
    std::unique_ptr<PdfDocument> pdfDoc;
    KCalCore::Calendar::Ptr calendar;
    std::unique_ptr<KMime::Message> mimeMsg;
    QJsonArray jsonResult;

    if (parser.value(typeOpt) == QLatin1String("pkpass")) {
        pass.reset(KPkPass::Pass::fromData(f.readAll()));
        engine.setPass(pass.get());
    } else if (parser.value(typeOpt) == QLatin1String("pdf")) {
        pdfDoc.reset(PdfDocument::fromData(f.readAll()));
        engine.setPdfDocument(pdfDoc.get());
    } else if (parser.value(typeOpt) == QLatin1String("html")) {
        htmlDoc.reset(HtmlDocument::fromData(f.readAll()));
        engine.setHtmlDocument(htmlDoc.get());
    } else if (parser.value(typeOpt) == QLatin1String("ical")) {
        calendar.reset(new KCalCore::MemoryCalendar(QTimeZone()));
        KCalCore::ICalFormat format;
        if (!format.fromRawString(calendar, f.readAll())) {
            std::cerr << "Failed to parse iCal file." << std::endl;
            return 1;
        }
        calendar->setProductId(format.loadedProductId());
        engine.setCalendar(calendar);
    } else if (parser.value(typeOpt) == QLatin1String("mime")) {
        mimeMsg.reset(new KMime::Message);
        mimeMsg->setContent(f.readAll());
        mimeMsg->parse();
        engine.setContent(mimeMsg.get());
    } else {
        engine.setData(f.readAll(), f.fileName());
    }

    jsonResult = engine.extract();
    const auto result = JsonLdDocument::fromJson(jsonResult);
    ExtractorPostprocessor postproc;
    postproc.setContextDate(contextDt);
    postproc.process(result);
    const auto postProcResult = JsonLdDocument::toJson(postproc.result());
    std::cout << QJsonDocument(postProcResult).toJson().constData() << std::endl;
}
