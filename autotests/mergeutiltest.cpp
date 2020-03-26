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

#include <KItinerary/Flight>
#include <KItinerary/JsonLdDocument>
#include <KItinerary/MergeUtil>
#include <KItinerary/Organization>
#include <KItinerary/Person>
#include <KItinerary/Reservation>
#include <KItinerary/TrainTrip>

#include <QDateTime>
#include <QDebug>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QObject>
#include <QTest>

#define _(x) QStringLiteral(x)

using namespace KItinerary;

class MergeUtilTest : public QObject
{
    Q_OBJECT
private:
    QByteArray readFile(const QString &fn) const
    {
        QFile f(fn);
        f.open(QFile::ReadOnly);
        return f.readAll();
    }

private Q_SLOTS:
    void testIsSameReservation()
    {
        QVERIFY(!MergeUtil::isSame({}, {}));
        FlightReservation res1;
        QVERIFY(!MergeUtil::isSame(res1, {}));
        QVERIFY(!MergeUtil::isSame({}, res1));

        res1.setReservationNumber(QStringLiteral("XXX007"));
        Flight flight1;
        flight1.setFlightNumber(QStringLiteral("1234"));
        flight1.setDepartureDay(QDate(2018, 4, 21));
        res1.setReservationFor(flight1);

        FlightReservation res2;
        res2.setReservationNumber(QStringLiteral("YYY008"));
        Flight flight2;
        flight2.setFlightNumber(QStringLiteral("1234"));
        res2.setReservationFor(flight2);
        QVERIFY(!MergeUtil::isSame(res1, res2));

        flight2.setDepartureDay(QDate(2018, 4, 21));
        res2.setReservationFor(flight2);
        QVERIFY(!MergeUtil::isSame(res1, res2));

        res2.setReservationNumber(QStringLiteral("XXX007"));
        QVERIFY(MergeUtil::isSame(res1, res2));
    }

    void testIsSameFlight()
    {
        Airline airline1;
        airline1.setIataCode(QStringLiteral("KL"));
        Flight f1;
        f1.setAirline(airline1);
        f1.setFlightNumber(QStringLiteral("8457"));
        f1.setDepartureTime(QDateTime(QDate(2018, 4, 2), QTime(17, 51, 0)));

        Flight f2;
        QVERIFY(!MergeUtil::isSame(f1, f2));

        f2.setFlightNumber(QStringLiteral("8457"));
        QVERIFY(!MergeUtil::isSame(f1, f2));

        Airline airline2;
        airline2.setIataCode(QStringLiteral("AF"));
        f2.setAirline(airline2);
        QVERIFY(!MergeUtil::isSame(f1, f2));
        airline2.setIataCode(QStringLiteral("KL"));
        f2.setAirline(airline2);
        QVERIFY(!MergeUtil::isSame(f1, f2));

        f2.setDepartureDay(QDate(2018, 4, 2));
        QVERIFY(MergeUtil::isSame(f1, f2));
    }

    void testCodeShareFlight()
    {
        Airline a1;
        a1.setIataCode(QStringLiteral("4U"));
        Flight f1;
        f1.setAirline(a1);
        f1.setFlightNumber(QStringLiteral("42"));
        f1.setDepartureDay(QDate(2018, 04, 21));

        Airline a2;
        a2.setIataCode(QStringLiteral("EW"));
        Flight f2(f1);
        f2.setAirline(a2);

        QVERIFY(MergeUtil::isSame(f1, f2));
    }

    void testIsSamePerson_data()
    {
        // we do not need to consider cases here that ExtractorPostprocessor eliminates for us
        // such as filling the full name or honoric prefixes
        QTest::addColumn<QVector<QStringList>>("data");

        QTest::newRow("simple name") << QVector<QStringList> {
            {_("Volker Krause"), {}, {}},
            {_("VOLKER KRAUSE"), {}, {}},
            {_("VOLKER KRAUSE"), _("Volker"), _("Krause")},
            {_("VOLKER KRAUSE"), {}, _("Krause")},
            {_("VOLKER KRAUSE"), _("Volker"), {}},
            // IATA BCBP artifacts
            {_("VOLKERMR KRAUSE"), _("VOLKERMR"), _("KRAUSE")},
            {_("VOLKER MR KRAUSE"), _("VOLKER MR"), _("KRAUSE")}
        };

        QTest::newRow("double family name") << QVector<QStringList> {
            {_("Andreas Cord-Landwehr"), {}, {}},
            {_("ANDREAS CORD-LANDWEHR"), {}, {}},
            {_("ANDREAS CORD-LANDWEHR"), _("Andreas"), _("Cord-Landwehr")},
            // IATA BCBP artifacts
            {_("Andreas Cordlandwehr"), {}, {}},
            {_("ANDREAS CORDLANDWEHR"), _("ANDREAS"), _("CORDLANDWEHR")},
            {_("ANDREAS CORD LANDWEHR"), _("ANDREAS"), _("CORD LANDWEHR")},
            {_("ANDREAS CORD LANDWEHR"), {}, {}}
        };

        QTest::newRow("diacritic") << QVector<QStringList> {
            {_("Daniel Vrátil"), {}, {} },
            {_("Daniel Vrátil"), _("Daniel"), _("Vrátil") },
            {_("DANIEL VRATIL"), {}, {} },
            {_("DANIEL VRATIL"), _("DANIEL"), _("VRATIL") }
        };
    }

    void testIsSamePerson()
    {
        QFETCH(QVector<QStringList>, data);

        for(int i = 0; i < data.size(); ++i) {
            Person lhs;
            lhs.setName(data[i][0]);
            lhs.setGivenName(data[i][1]);
            lhs.setFamilyName(data[i][2]);

            for (int j = 0; j < data.size(); ++j) {
                Person rhs;
                rhs.setName(data[j][0]);
                rhs.setGivenName(data[j][1]);
                rhs.setFamilyName(data[j][2]);

                QVERIFY(!MergeUtil::isSamePerson(lhs, {}));
                QVERIFY(!MergeUtil::isSamePerson({}, lhs));

                if (!MergeUtil::isSamePerson(lhs, rhs)) {
                    qDebug() << "Left: " << lhs.name() << lhs.givenName() << lhs.familyName();
                    qDebug() << "Right: " << rhs.name() << rhs.givenName() << rhs.familyName();
                }
                QVERIFY(MergeUtil::isSamePerson(lhs, rhs));
            }
        }
    }

    void testIsNotSamePerson()
    {
        QVector<QStringList> data {
            { _("Volker Krause"), {}, {} },
            { _("Andreas Cord-Landwehr"), _("Andread"), _("Cord-Landwehr") },
            { _("GIVEN1 GIVEN2 FAMILY1"), {}, {} },
            { _("V K"), {}, {} },
            {_("Daniel Vrátil"), _("Daniel"), _("Vrátil") },
        };

        for(int i = 0; i < data.size(); ++i) {
            Person lhs;
            lhs.setName(data[i][0]);
            lhs.setGivenName(data[i][1]);
            lhs.setFamilyName(data[i][2]);

            for (int j = 0; j < data.size(); ++j) {
                if (i == j) {
                    continue;
                }

                Person rhs;
                rhs.setName(data[j][0]);
                rhs.setGivenName(data[j][1]);
                rhs.setFamilyName(data[j][2]);

                QVERIFY(!MergeUtil::isSamePerson(lhs, {}));
                QVERIFY(!MergeUtil::isSamePerson({}, lhs));

                if (MergeUtil::isSamePerson(lhs, rhs)) {
                    qDebug() << "Left: " << lhs.name() << lhs.givenName() << lhs.familyName();
                    qDebug() << "Right: " << rhs.name() << rhs.givenName() << rhs.familyName();
                }
                QVERIFY(!MergeUtil::isSamePerson(lhs, rhs));
            }
        }
    }

    void testIsSameTrain_data()
    {
        QTest::addColumn<QString>("lhsName");
        QTest::addColumn<QString>("lhsNumber");
        QTest::addColumn<QString>("rhsName");
        QTest::addColumn<QString>("rhsNumber");
        QTest::addColumn<bool>("shouldBeSame");

        QTest::newRow("empty") << QString() << QString() << QString() << QString() << false;
        QTest::newRow("equal number") << QString() << _("123") << QString() << _("123") << true;
        QTest::newRow("optional name") << QString() << _("123") << _("EC") << _("123") << true;
        QTest::newRow("optional name2") << QString() << _("123") << QString() << _("EC 123") << true;
        QTest::newRow("name mismatch") << _("IC") << _("123") << _("EC") << _("123") << false;
        QTest::newRow("name mismatch2") << QString() << _("IC 123") << QString() << _("EC 123") << false;
        QTest::newRow("number mismatch") << QString() << _("123") << QString() << _("234") << false;
        QTest::newRow("number prefix") << QString() << _("123") << QString() << _("12") << false;
        QTest::newRow("number prefix2") << QString() << _("123") << QString() << _("23") << false;
    }

    void testIsSameTrain()
    {
        QFETCH(QString, lhsName);
        QFETCH(QString, lhsNumber);
        QFETCH(QString, rhsName);
        QFETCH(QString, rhsNumber);
        QFETCH(bool, shouldBeSame);

        TrainTrip lhs;
        lhs.setDepartureTime(QDateTime({2019, 11, 9}, {12, 00}));
        lhs.setTrainName(lhsName);
        lhs.setTrainNumber(lhsNumber);

        TrainTrip rhs;
        rhs.setDepartureTime(QDateTime({2019, 11, 9}, {12, 00}));
        rhs.setTrainName(rhsName);
        rhs.setTrainNumber(rhsNumber);

        QCOMPARE(MergeUtil::isSame(lhs, rhs), shouldBeSame);
        QCOMPARE(MergeUtil::isSame(rhs, lhs), shouldBeSame);
    }

    void testIsSameLodingReservation()
    {
        LodgingReservation res1;
        LodgingBusiness hotel1;
        hotel1.setName(QStringLiteral("Haus Randa"));
        res1.setReservationFor(hotel1);
        res1.setCheckinTime(QDateTime(QDate(2018, 4, 9), QTime(10, 0)));
        res1.setReservationNumber(QStringLiteral("1234"));

        LodgingReservation res2;
        QVERIFY(!MergeUtil::isSame(res1, res2));
        res2.setReservationNumber(QStringLiteral("1234"));
        QVERIFY(!MergeUtil::isSame(res1, res2));
        res2.setCheckinTime(QDateTime(QDate(2018, 4, 9), QTime(15, 0)));
        QVERIFY(!MergeUtil::isSame(res1, res2));
        LodgingBusiness hotel2;
        hotel2.setName(QStringLiteral("Haus Randa"));
        res2.setReservationFor(hotel2);
        QVERIFY(MergeUtil::isSame(res1, res2));
    }

    void testIsSameCancellation()
    {
        const auto lhs = JsonLdDocument::fromJson(QJsonDocument::fromJson(readFile(_(SOURCE_DIR "/mergedata/cancellation.lhs.json"))).array());
        const auto rhs = JsonLdDocument::fromJson(QJsonDocument::fromJson(readFile(_(SOURCE_DIR "/mergedata/cancellation.rhs.json"))).array());
        QCOMPARE(lhs.size(), 1);
        QCOMPARE(rhs.size(), 1);
        QVERIFY(MergeUtil::isSame(lhs[0], rhs[0]));
    }

    void testMerge_data()
    {
        QTest::addColumn<QString>("baseName");

        QDir dir(QStringLiteral(SOURCE_DIR "/mergedata"));
        const auto lst = dir.entryList(QStringList(QStringLiteral("*.merged.json")), QDir::Files | QDir::Readable | QDir::NoSymLinks);
        for (const auto &file : lst) {
            const auto base = file.left(file.size() - 12);
            QTest::newRow(base.toLatin1().constData()) << (dir.path() + QLatin1Char('/') + base);
        }
    }

    void testMerge()
    {
        QFETCH(QString, baseName);

        const auto lhs = JsonLdDocument::fromJson(QJsonDocument::fromJson(readFile(baseName + QLatin1String(".lhs.json"))).array()).first();
        const auto rhs = JsonLdDocument::fromJson(QJsonDocument::fromJson(readFile(baseName + QLatin1String(".rhs.json"))).array()).first();
        const auto expected = QJsonDocument::fromJson(readFile(baseName + QLatin1String(".merged.json")));

        const auto mergedL2R = MergeUtil::merge(lhs, rhs);
        auto mergedJson = QJsonDocument(JsonLdDocument::toJson(QVector<QVariant>({mergedL2R})));
        if (mergedJson != expected) {
            qDebug().noquote() << mergedJson.toJson();
        }
        QCOMPARE(mergedJson, expected);

        const auto mergedR2L = MergeUtil::merge(rhs, lhs);
        mergedJson = QJsonDocument(JsonLdDocument::toJson(QVector<QVariant>({mergedR2L})));
        if (mergedJson != expected) {
            qDebug().noquote() << mergedJson.toJson();
        }
        QCOMPARE(mergedJson, expected);
    }
};

QTEST_APPLESS_MAIN(MergeUtilTest)

#include "mergeutiltest.moc"
