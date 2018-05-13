/*
  Copyright (c) 2018 Volker Krause <vkrause@kde.org>

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

#include <KItinerary/TrainStationDb>

#include <QDebug>
#include <QObject>
#include <QTest>
#include <QTimeZone>

using namespace KItinerary;
using namespace KItinerary::KnowledgeDb;

class KnowledgeDbTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testIBNRLookup()
    {
        auto station = TrainStationDb::stationForIbnr(IBNR{1234567});
        QVERIFY(!station.coordinate.isValid());
        QCOMPARE(station.timezone.toQTimeZone(), QTimeZone());

        station = TrainStationDb::stationForIbnr({});
        QVERIFY(!station.coordinate.isValid());
        QCOMPARE(station.timezone.toQTimeZone(), QTimeZone());

        station = TrainStationDb::stationForIbnr(IBNR{8011160});
        QVERIFY(station.coordinate.isValid());
        QCOMPARE(station.timezone.toQTimeZone(), QTimeZone("Europe/Berlin"));

        station = TrainStationDb::stationForIbnr(IBNR{8501687});
        QVERIFY(station.coordinate.isValid());
        QCOMPARE(station.timezone.toQTimeZone(), QTimeZone("Europe/Zurich"));
    }

    void testGaresConnexionsIdLookup()
    {
        auto station = TrainStationDb::stationForGaresConnexionsId({});
        QVERIFY(!station.coordinate.isValid());
        QCOMPARE(station.timezone.toQTimeZone(), QTimeZone());

        station = TrainStationDb::stationForGaresConnexionsId(GaresConnexionsId{"XXXXX"});
        QVERIFY(!station.coordinate.isValid());
        QCOMPARE(station.timezone.toQTimeZone(), QTimeZone());

        station = TrainStationDb::stationForGaresConnexionsId(GaresConnexionsId{"FRAES"});
        QVERIFY(station.coordinate.isValid());
        QCOMPARE(station.timezone.toQTimeZone(), QTimeZone("Europe/Paris"));

        station = TrainStationDb::stationForGaresConnexionsId(GaresConnexionsId{QStringLiteral("FRXYT")});
        QVERIFY(station.coordinate.isValid());
        QCOMPARE(station.timezone.toQTimeZone(), QTimeZone("Europe/Paris"));

        station = TrainStationDb::stationForGaresConnexionsId(GaresConnexionsId{"CHGVA"});
        QEXPECT_FAIL("", "Wikidata does not supply ids for non-French stations yet", Continue);
        QVERIFY(station.coordinate.isValid());
        QEXPECT_FAIL("", "Wikidata does not supply ids for non-French stations yet", Continue);
        QCOMPARE(station.timezone.toQTimeZone(), QTimeZone("Europe/Zurich"));
    }
};

QTEST_APPLESS_MAIN(KnowledgeDbTest)

#include "knowledgedbtest.moc"
