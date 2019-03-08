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
#include <KItinerary/JsonLdDocument>

#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDateTime>
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

    ExtractorEngine engine;
    ExtractorPostprocessor postproc;

    auto contextDt = QDateTime::fromString(parser.value(ctxOpt), Qt::ISODate);
    if (!contextDt.isValid()) {
        contextDt = QDateTime::currentDateTime();
    }
    postproc.setContextDate(contextDt);

    const auto files = parser.positionalArguments().isEmpty() ? QStringList(QString()) : parser.positionalArguments();
    for (const auto arg : files) {
        QFile f;
        if (!arg.isEmpty()) {
            f.setFileName(arg);
            if (!f.open(QFile::ReadOnly)) {
                std::cerr << qPrintable(f.errorString()) << std::endl;
                return 1;
            }
        } else {
            f.open(stdin, QFile::ReadOnly);
        }

        auto fileName = f.fileName();
        const auto typeArg = parser.value(typeOpt);
        if (!typeArg.isEmpty()) {
            if (typeArg == QLatin1String("mime")) {
                fileName = QStringLiteral("dummy.eml");
            } else {
                fileName = QLatin1String("dummy.") + typeArg;
            }
        }

        engine.clear();
        engine.setContextDate(contextDt);
        engine.setData(f.readAll(), fileName);

        const auto result = JsonLdDocument::fromJson(engine.extract());
        postproc.process(result);
    }

    const auto postProcResult = JsonLdDocument::toJson(postproc.result());
    std::cout << QJsonDocument(postProcResult).toJson().constData() << std::endl;
}
