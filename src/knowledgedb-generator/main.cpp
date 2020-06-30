/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "airportdbgenerator.h"
#include "countrydbgenerator.h"
#include "timezonedbgenerator.h"
#include "trainstationdbgenerator.h"

#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDebug>
#include <QFile>

using namespace KItinerary::Generator;

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    QCommandLineParser parser;
    QCommandLineOption dbOpt({QStringLiteral("d"), QStringLiteral("database")}, QStringLiteral("The database to generate."), QStringLiteral("database type"));
    parser.addOption(dbOpt);
    QCommandLineOption outputOpt({QStringLiteral("o"), QStringLiteral("output")}, QStringLiteral("Output file."), QStringLiteral("output file"));
    parser.addOption(outputOpt);
    QCommandLineOption osmOpt({QStringLiteral("m"), QStringLiteral("osm-data")}, QStringLiteral("OSM data file."), QStringLiteral("osm file"));
    parser.addOption(osmOpt);
    parser.addHelpOption();
    parser.process(app);

    QFile out(parser.value(outputOpt));
    if (!out.open(QFile::WriteOnly)) {
        qWarning() << out.errorString();
        return 1;
    }

    if (parser.value(dbOpt) == QLatin1String("airport")) {
        AirportDbGenerator gen;
        gen.osmDb.load(parser.value(osmOpt));
        return gen.generate(&out) ? 0 : 1;
    } else if (parser.value(dbOpt) == QLatin1String("country")) {
        CountryDbGenerator gen;
        return gen.generate(&out) ? 0 : 1;
    } else if (parser.value(dbOpt) == QLatin1String("timezone")) {
        TimezoneDbGenerator gen;
        gen.generate(&out);
    } else if (parser.value(dbOpt) == QLatin1String("timezoneheader")) {
        TimezoneDbGenerator gen;
        gen.generateHeader(&out);
    } else if (parser.value(dbOpt) == QLatin1String("trainstation")) {
        TrainStationDbGenerator gen;
        return gen.generate(&out) ? 0 : 1;
    }
}
