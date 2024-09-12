/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <config-kitinerary.h>
#include <kitinerary_version.h>

#include <KItinerary/CalendarHandler>
#include <KItinerary/ExtractorCapabilities>
#include <KItinerary/ExtractorEngine>
#include <KItinerary/ExtractorPostprocessor>
#include <KItinerary/ExtractorRepository>
#include <KItinerary/ExtractorValidator>
#include <KItinerary/JsonLdDocument>
#include <KItinerary/MergeUtil>
#include <KItinerary/Reservation>
#include <KItinerary/ScriptExtractor>

#include <KCalendarCore/Event>
#include <KCalendarCore/ICalFormat>
#include <KCalendarCore/MemoryCalendar>

#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QObject>

#include <iostream>

using namespace KItinerary;

static QList<QList<QVariant>>
batchReservations(const QList<QVariant> &reservations) {
  using namespace KItinerary;

  QList<QList<QVariant>> batches;
  QList<QVariant> batch;

  for (const auto &res : reservations) {
    if (batch.isEmpty()) {
      batch.push_back(res);
      continue;
    }

    if (JsonLd::canConvert<Reservation>(res) &&
        JsonLd::canConvert<Reservation>(batch.at(0))) {
      const auto trip1 = JsonLd::convert<Reservation>(res).reservationFor();
      const auto trip2 =
          JsonLd::convert<Reservation>(batch.at(0)).reservationFor();
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
    for (const auto &ext : repo.extractors()) {
        std::cout << qPrintable(ext->name());
        if (auto scriptExt = dynamic_cast<const ScriptExtractor*>(ext.get())) {
            std::cout << " (" << qPrintable(scriptExt->mimeType()) << ", "
                      << qPrintable(scriptExt->scriptFileName()) << ":"
                      << qPrintable(scriptExt->scriptFunction()) << ")";
        }
        std::cout << std::endl;
    }
}

int main(int argc, char** argv)
{
    QCoreApplication::setApplicationName(QStringLiteral("kitinerary-extractor"));
    QCoreApplication::setApplicationVersion(QStringLiteral(KITINERARY_VERSION_STRING));
    QCoreApplication::setOrganizationDomain(QStringLiteral("kde.org"));
    QCoreApplication::setOrganizationName(QStringLiteral("KDE"));
    QCoreApplication app(argc, argv);

#ifdef KITINERARY_STANDALONE_CLI_EXTRACTOR
    // set additional data file search path relative to the current binary location
    // NOTE: QCoreApplication::applicationDirPath is only valid once QCoreApplication has been created
    auto xdgDataDirs = qgetenv("XDG_DATA_DIRS");
    if (!xdgDataDirs.isEmpty()) {
        xdgDataDirs += QDir::listSeparator().toLatin1();
    }
    xdgDataDirs += QString(QCoreApplication::applicationDirPath() +
                           QDir::separator() + QLatin1StringView("..") +
                           QDir::separator() + QLatin1StringView("share"))
                       .toUtf8();
    qputenv("XDG_DATA_DIRS", xdgDataDirs);
#endif

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
    QCommandLineOption typeOpt({QStringLiteral("t"), QStringLiteral("type")}, QStringLiteral("Deprecated, no longer needed and ignored."), QStringLiteral("type"));
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

    const auto files = parser.positionalArguments().isEmpty() ? QStringList(QString()) : parser.positionalArguments();
    for (const auto &arg : files) {
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

        engine.clear();
        engine.setContextDate(contextDt);

        if (!parser.value(extOpt).isEmpty()) {
            const auto extNames = parser.value(extOpt).split(QLatin1Char(';'),
                                                             Qt::SkipEmptyParts);
            std::vector<const AbstractExtractor*> exts;
            exts.reserve(extNames.size());
            for (const auto &name : extNames) {
                const auto ext = repo.extractorByName(name);
                exts.push_back(ext);
            }
            engine.setAdditionalExtractors(std::move(exts));
        }

        engine.setData(f.readAll(), fileName);
        const auto result = JsonLdDocument::fromJson(engine.extract());
        postproc.process(result);
    }

    auto result = postproc.result();
    if (!parser.isSet(noValidationOpt)) {
        ExtractorValidator validator;
        result.erase(std::remove_if(result.begin(), result.end(), [&validator](const auto &elem) {
            return !validator.isValidElement(elem);
        }), result.end());
    }

    if (parser.value(formatOpt).compare(QLatin1StringView("ical"),
                                        Qt::CaseInsensitive) == 0) {
      const auto batches = batchReservations(result);
      KCalendarCore::Calendar::Ptr cal(
          new KCalendarCore::MemoryCalendar(QTimeZone::systemTimeZone()));
      for (const auto &batch : batches) {
        KCalendarCore::Event::Ptr event(new KCalendarCore::Event);
        CalendarHandler::fillEvent(batch, event);
        cal->addEvent(event);
      }
      KCalendarCore::ICalFormat format;
      std::cout << qPrintable(format.toString(cal));
    } else {
      const auto postProcResult = JsonLdDocument::toJson(result);
      std::cout << QJsonDocument(postProcResult).toJson().constData()
                << std::endl;
    }
}
