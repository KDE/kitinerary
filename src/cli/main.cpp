/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <config-kitinerary.h>
#include <kitinerary_version.h>

#include <KItinerary/CalendarHandler>
#include <KItinerary/Extractor>
#include <KItinerary/ExtractorCapabilities>
#include <KItinerary/ExtractorEngine>
#include <KItinerary/ExtractorInput>
#include <KItinerary/ExtractorPostprocessor>
#include <KItinerary/ExtractorRepository>
#include <KItinerary/JsonLdDocument>
#include <KItinerary/MergeUtil>
#include <KItinerary/Reservation>

#include <KCalendarCore/Event>
#include <KCalendarCore/ICalFormat>
#include <KCalendarCore/MemoryCalendar>

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

static QVector<QVector<QVariant>> batchReservations(const QVector<QVariant> &reservations)
{
    using namespace KItinerary;

    QVector<QVector<QVariant>> batches;
    QVector<QVariant> batch;

    for (const auto &res : reservations) {
        if (batch.isEmpty()) {
            batch.push_back(res);
            continue;
        }

        if (JsonLd::canConvert<Reservation>(res) && JsonLd::canConvert<Reservation>(batch.at(0))) {
            const auto trip1 = JsonLd::convert<Reservation>(res).reservationFor();
            const auto trip2 = JsonLd::convert<Reservation>(batch.at(0)).reservationFor();
            if (KItinerary::MergeUtil::isSame(trip1, trip2)) {
                batch.push_back(res);
                continue;
            }
        }

        batches.push_back(batch);
        batch.clear();
        batch.push_back(res);
    }

    if (!batch.isEmpty()) {
        batches.push_back(batch);
    }
    return batches;
}

static void printCapabilities()
{
    std::cout << qPrintable(ExtractorCapabilities::capabilitiesString());
}

static void printExtractors()
{
    ExtractorRepository repo;
    for (const auto &ext : repo.allExtractors()) {
        std::cout << qPrintable(ext.name()) << " (" << qPrintable(ExtractorInput::typeToString(ext.type()));
        if (!ext.scriptFileName().isEmpty()) {
            std::cout << ", " << qPrintable(ext.scriptFileName()) << ":" << qPrintable(ext.scriptFunction());
        }
        std::cout << ")" << std::endl;
    }
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
    QCommandLineOption listExtOpt({QStringLiteral("list-extractors")}, QStringLiteral("List all available extractors."));
    parser.addOption(listExtOpt);

    QCommandLineOption ctxOpt({QStringLiteral("c"), QStringLiteral("context-date")}, QStringLiteral("ISO date/time for when this data has been received."), QStringLiteral("date"));
    parser.addOption(ctxOpt);
    QCommandLineOption typeOpt({QStringLiteral("t"), QStringLiteral("type")}, QStringLiteral("Type of the input data [Email, Pdf, PkPass, ICal, Html]."), QStringLiteral("type"));
    parser.addOption(typeOpt);
    QCommandLineOption extOpt({QStringLiteral("e"), QStringLiteral("extractors")}, QStringLiteral("Additional extractors to apply."), QStringLiteral("extractors"));
    parser.addOption(extOpt);
    QCommandLineOption pathsOpt({QStringLiteral("additional-search-path")}, QStringLiteral("Additional search path for extractors."), QStringLiteral("search-path"));
    parser.addOption(pathsOpt);
    QCommandLineOption formatOpt({QStringLiteral("o"), QStringLiteral("output")}, QStringLiteral("Output format [JsonLd, iCal]. Default: JsonLd"), QStringLiteral("format"));
    parser.addOption(formatOpt);
    QCommandLineOption noValidationOpt({QStringLiteral("no-validation")}, QStringLiteral("Disable result validation."));
    parser.addOption(noValidationOpt);

    parser.addPositionalArgument(QStringLiteral("input"), QStringLiteral("File to extract data from, omit for using stdin."));
    parser.process(app);

    ExtractorRepository repo;
    if (parser.isSet(pathsOpt)) {
        repo.setAdditionalSearchPaths(parser.values(pathsOpt));
        repo.reload();
    }

    if (parser.isSet(capOpt)) {
        printCapabilities();
        return 0;
    }
    if (parser.isSet(listExtOpt)) {
        printExtractors();
        return 0;
    }

    ExtractorEngine engine;
    engine.setUseSeparateProcess(false); // we are the external extractor
    ExtractorPostprocessor postproc;

    auto contextDt = QDateTime::fromString(parser.value(ctxOpt), Qt::ISODate);
    if (!contextDt.isValid()) {
        contextDt = QDateTime::currentDateTime();
    }
    postproc.setContextDate(contextDt);
    postproc.setValidationEnabled(!parser.isSet(noValidationOpt));

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
        const auto typeArg = ExtractorInput::typeFromName(parser.value(typeOpt));

        engine.clear();
        engine.setContextDate(contextDt);

        if (!parser.value(extOpt).isEmpty()) {
            const auto extNames = parser.value(extOpt).split(QLatin1Char(';'),
                                                             Qt::SkipEmptyParts);
            std::vector<Extractor> exts;
            exts.reserve(extNames.size());
            for (const auto &name : extNames) {
                const auto ext = repo.extractor(name);
                exts.push_back(ext);
            }
            engine.setAdditionalExtractors(std::move(exts));
        }

        if (typeArg == ExtractorInput::Unknown) {
            engine.setData(f.readAll(), fileName);
        } else {
            engine.setData(f.readAll(), typeArg);
        }

        const auto result = JsonLdDocument::fromJson(engine.extract());
        postproc.process(result);
    }


    if (ExtractorInput::typeFromName(parser.value(formatOpt)) == ExtractorInput::ICal) {
        const auto batches = batchReservations(postproc.result());
        KCalendarCore::Calendar::Ptr cal(new KCalendarCore::MemoryCalendar(QTimeZone::systemTimeZone()));
        for (const auto &batch : batches) {
            KCalendarCore::Event::Ptr event(new KCalendarCore::Event);
            CalendarHandler::fillEvent(batch, event);
            cal->addEvent(event);
        }
        KCalendarCore::ICalFormat format;
        std::cout << qPrintable(format.toString(cal));
    } else {
        const auto postProcResult = JsonLdDocument::toJson(postproc.result());
        std::cout << QJsonDocument(postProcResult).toJson().constData() << std::endl;
    }
}
