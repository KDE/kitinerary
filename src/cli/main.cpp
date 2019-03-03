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
    // TODO
}

int main(int argc, char** argv)
{
    QCoreApplication::setApplicationName(QStringLiteral("kitinerary-extractor"));
    QCoreApplication::setApplicationVersion(QStringLiteral("TODO"));
    QCoreApplication::setOrganizationDomain(QStringLiteral("kde.org"));
    QCoreApplication::setOrganizationName(QStringLiteral("KDE"));
    QCoreApplication app(argc, argv);

    QCommandLineParser parser;
    parser.setApplicationDescription(QStringLiteral("Command line itinerary extractor."));
    parser.addHelpOption();
    parser.addVersionOption();
    QCommandLineOption capOpt({QStringLiteral("capabilities")}, QStringLiteral("Show available extraction capabilities."));
    parser.addOption(capOpt);
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

    ExtractorEngine engine;

    std::unique_ptr<KPkPass::Pass> pass;
    std::unique_ptr<HtmlDocument> htmlDoc;
    std::unique_ptr<PdfDocument> pdfDoc;
    KCalCore::Calendar::Ptr calendar;
    std::unique_ptr<KMime::Message> mimeMsg;
    QJsonArray jsonResult;

    if (f.fileName().endsWith(QLatin1String(".pkpass")) || parser.value(typeOpt) == QLatin1String("pkpass")) {
        pass.reset(KPkPass::Pass::fromData(f.readAll()));
        engine.setPass(pass.get());
    } else if (f.fileName().endsWith(QLatin1String(".pdf")) || parser.value(typeOpt) == QLatin1String("pdf")) {
        pdfDoc.reset(PdfDocument::fromData(f.readAll()));
        engine.setPdfDocument(pdfDoc.get());
    } else if (f.fileName().endsWith(QLatin1String(".html")) || parser.value(typeOpt) == QLatin1String("html")) {
        htmlDoc.reset(HtmlDocument::fromData(f.readAll()));
        engine.setHtmlDocument(htmlDoc.get());
    } else if (f.fileName().endsWith(QLatin1String(".txt"))) {
        engine.setText(QString::fromUtf8(f.readAll()));
    } else if (f.fileName().endsWith(QLatin1String(".ics")) || parser.value(typeOpt) == QLatin1String("ical")) {
        calendar.reset(new KCalCore::MemoryCalendar(QTimeZone()));
        KCalCore::ICalFormat format;
        if (!format.fromRawString(calendar, f.readAll())) {
            std::cerr << "Failed to parse iCal file." << std::endl;
            return 1;
        }
        calendar->setProductId(format.loadedProductId());
        engine.setCalendar(calendar);
    } else if (f.fileName().endsWith(QLatin1String(".eml")) || f.fileName().endsWith(QLatin1String(".mbox")) || parser.value(typeOpt) == QLatin1String("mime")) {
        mimeMsg.reset(new KMime::Message);
        mimeMsg->setContent(f.readAll());
        mimeMsg->parse();
        engine.setContent(mimeMsg.get());
    }

    jsonResult = engine.extract();
    const auto result = JsonLdDocument::fromJson(jsonResult);
    ExtractorPostprocessor postproc;
    // TODO extra option to set this, or retrieve from mime message
//     postproc.setContextDate(contextMsg.date()->dateTime());
    postproc.process(result);
    const auto postProcResult = JsonLdDocument::toJson(postproc.result());
    std::cout << QJsonDocument(postProcResult).toJson().constData() << std::endl;
}
