/*
    Copyright (C) 2018 Volker Krause <vkrause@kde.org>

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
    parser.addHelpOption();
    parser.process(app);

    QFile out(parser.value(outputOpt));
    if (!out.open(QFile::WriteOnly)) {
        qWarning() << out.errorString();
        return 1;
    }

    if (parser.value(dbOpt) == QLatin1String("airport")) {
        AirportDbGenerator gen;
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
