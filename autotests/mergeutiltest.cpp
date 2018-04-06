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
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <KItinerary/Flight>
#include <KItinerary/MergeUtil>
#include <KItinerary/Organization>
#include <KItinerary/Person>
#include <KItinerary/Reservation>

#include <QDebug>
#include <QObject>
#include <QTest>

using namespace KItinerary;

class MergeUtilTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testIsSameReservation()
    {
        QVERIFY(!MergeUtil::isSameReservation({}, {}));
        FlightReservation res1;
        QVERIFY(!MergeUtil::isSameReservation(QVariant::fromValue(res1), {}));
        QVERIFY(!MergeUtil::isSameReservation({}, QVariant::fromValue(res1)));

        res1.setReservationNumber(QLatin1String("XXX007"));
        Flight flight1;
        flight1.setFlightNumber(QLatin1String("1234"));
        res1.setReservationFor(QVariant::fromValue(flight1));

        FlightReservation res2;
        res2.setReservationNumber(QLatin1String("YYY008"));
        Flight flight2;
        flight2.setFlightNumber(QLatin1String("1234"));
        res2.setReservationFor(QVariant::fromValue(flight2));
        QVERIFY(!MergeUtil::isSameReservation(QVariant::fromValue(res1), QVariant::fromValue(res2)));

        res2.setReservationNumber(QLatin1String("XXX007"));
        QVERIFY(MergeUtil::isSameReservation(QVariant::fromValue(res1), QVariant::fromValue(res2)));
    }

    void testIsSameFlight()
    {
        Airline airline1;
        airline1.setIataCode(QLatin1String("KL"));
        Flight f1;
        f1.setAirline(airline1);
        f1.setFlightNumber(QLatin1String("8457"));
        f1.setDepartureTime(QDateTime(QDate(2018, 4, 2), QTime(17, 51, 0)));

        Flight f2;
        QVERIFY(!MergeUtil::isSameFlight(f1, f2));

        f2.setFlightNumber(QLatin1String("8457"));
        QVERIFY(!MergeUtil::isSameFlight(f1, f2));

        Airline airline2;
        airline2.setIataCode(QLatin1String("AF"));
        f2.setAirline(airline2);
        QVERIFY(!MergeUtil::isSameFlight(f1, f2));
        airline2.setIataCode(QLatin1String("KL"));
        f2.setAirline(airline2);
        QVERIFY(!MergeUtil::isSameFlight(f1, f2));

        f2.setDepartureDay(QDate(2018, 4, 2));
        QVERIFY(MergeUtil::isSameFlight(f1, f2));
    }

    void testIsSamePerson()
    {
        Person p1;
        p1.setName(QLatin1String("VOLKER KRAUSE"));
        QVERIFY(!MergeUtil::isSamePerson(p1, {}));
        QVERIFY(!MergeUtil::isSamePerson({}, p1));
        QVERIFY(MergeUtil::isSamePerson(p1, p1));

        Person p2;
        p2.setName(QLatin1String("Volker Krause"));
        QVERIFY(MergeUtil::isSamePerson(p1, p2));
    }
};

QTEST_APPLESS_MAIN(MergeUtilTest)

#include "mergeutiltest.moc"
