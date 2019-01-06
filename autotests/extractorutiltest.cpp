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

#include <extractorutil.h>

#include <KItinerary/Flight>
#include <KItinerary/Place>

#include <QDebug>
#include <QTest>

using namespace KItinerary;

#define s(x) QStringLiteral(x)

class ExtractorUtilTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testTerminalExtraction_data()
    {
        QTest::addColumn<QString>("input");
        QTest::addColumn<QString>("airportName");
        QTest::addColumn<QString>("terminalName");

        QTest::newRow("empty") << QString() << QString() << QString();
        QTest::newRow("no terminal") << s("Paris Charles de Gaulle") << s("Paris Charles de Gaulle") << s("");
        QTest::newRow("CDG 1") << s("PARIS, FR (CHARLES DE GAULLE), TERMINAL 2E") << s("PARIS, FR (CHARLES DE GAULLE)") << s("2E");
        QTest::newRow("CDG 2") << s("Paris Charles de Gaulle (Terminal 2D)") << s("Paris Charles de Gaulle") << s("2D");
        QTest::newRow("LHR") << s("London/Heathrow-Terminal 2") << s("London/Heathrow") << s("2");
        QTest::newRow("MAD") << s("MADRID, ES (BARAJAS), TERMINAL 4S") << s("MADRID, ES (BARAJAS)") << s("4S");
        QTest::newRow("DTW") << s("DETROIT, MI (METROPOLITAN WAYNE CO), TERMINAL EM") << s("DETROIT, MI (METROPOLITAN WAYNE CO)") << s("EM");
        QTest::newRow("MRS") << s("MARSEILLE FR - PROVENCE - TERMINAL 1A") << s("MARSEILLE FR - PROVENCE") << s("1A");
        QTest::newRow("MUC") << s("München (Terminal 1)") << s("München") << s("1");
        QTest::newRow("GTW EN") << s("London Gatwick (North Terminal)") << s("London Gatwick") << s("North");
        QTest::newRow("GTW DE") << s("London Gatwick (Terminal Nord)") << s("London Gatwick") << s("Nord");
        QTest::newRow("TXL") << s("Berlin Tegel (Terminal C)") << s("Berlin Tegel") << s("C");
    }

    void testTerminalExtraction()
    {
        QFETCH(QString, input);
        QFETCH(QString, airportName);
        QFETCH(QString, terminalName);

        Airport a;
        a.setName(input);
        Flight f;
        f.setDepartureAirport(a);
        f.setArrivalAirport(a);

        const auto out = ExtractorUtil::extractTerminals(f);
        QCOMPARE(out.departureAirport().name(), airportName);
        QCOMPARE(out.departureTerminal(), terminalName);
        QCOMPARE(out.arrivalAirport().name(), airportName);
        QCOMPARE(out.arrivalTerminal(), terminalName);

        const auto out2 = ExtractorUtil::extractTerminals(out);
        QCOMPARE(out, out2);
    }

    void testPostalCodeExtraction_data()
    {
        QTest::addColumn<QString>("input");
        QTest::addColumn<QString>("city");
        QTest::addColumn<QString>("postalCode");

        QTest::newRow("empty") << QString() << QString() << QString();
        QTest::newRow("no code") << s("PARIS") << s("PARIS") << s("");
        QTest::newRow("prefix 1") << s("1060 Brussels") << s("Brussels") << s("1060");
        QTest::newRow("prefix 2") << s("171060 Brussels") << s("Brussels") << s("171060");
        QTest::newRow("prefix 3") << s("75012 Paris Some Suffix") << s("Paris Some Suffix") << s("75012");
    }

    void testPostalCodeExtraction()
    {
        QFETCH(QString, input);
        QFETCH(QString, city);
        QFETCH(QString, postalCode);

        PostalAddress a;
        a.setAddressLocality(input);

        const auto out = ExtractorUtil::extractPostalCode(a);
        QCOMPARE(out.addressLocality(), city);
        QCOMPARE(out.postalCode(), postalCode);

        const auto out2 = ExtractorUtil::extractPostalCode(out);
        QCOMPARE(out, out2);
    }
};

QTEST_GUILESS_MAIN(ExtractorUtilTest)

#include "extractorutiltest.moc"
