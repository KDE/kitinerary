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

#include <KItinerary/Flight>
#include <KItinerary/JsonLdDocument>
#include <KItinerary/Organization>
#include <KItinerary/Place>
#include <KItinerary/Reservation>
#include <KItinerary/Person>

#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QObject>
#include <QTest>
#include <QTimeZone>

using namespace KItinerary;

class JsonLdDocumentTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase()
    {
        qputenv("TZ", "GMT");
    }

    void testSerialization()
    {
        Flight f;
        f.setFlightNumber(QLatin1String("1234"));
        f.setDepartureTime(QDateTime(QDate(2018, 3, 18), QTime(18, 44, 0), QTimeZone("Europe/Berlin")));
        f.setDepartureDay(QDate(2018, 3, 18));
        f.setArrivalTime(QDateTime(QDate(2018, 3, 18), QTime(19, 44, 0), Qt::UTC));
        Airport ap;
        ap.setName(QLatin1String("Berlin Tegel"));
        ap.setIataCode(QLatin1String("TXL"));
        f.setDepartureAirport(ap);
        f.setDepartureGate(QLatin1String(""));
        Airline airline;
        airline.setIataCode(QLatin1String("LH"));
        f.setAirline(airline);

        auto array = JsonLdDocument::toJson({QVariant::fromValue(f)});
        QCOMPARE(array.size(), 1);
        auto obj = array.at(0).toObject();
        QCOMPARE(obj.value(QLatin1String("@context")).toString(), QLatin1String("http://schema.org"));
        QCOMPARE(obj.value(QLatin1String("@type")).toString(), QLatin1String("Flight"));
        QCOMPARE(obj.value(QLatin1String("flightNumber")).toString(), QLatin1String("1234"));

        QCOMPARE(obj.value(QLatin1String("arrivalTime")).toString(), QLatin1String("2018-03-18T19:44:00Z"));
        auto dtObj = obj.value(QLatin1String("departureTime")).toObject();
        QCOMPARE(dtObj.value(QLatin1String("@value")).toString(), QLatin1String("2018-03-18T18:44:00+01:00"));
        QCOMPARE(dtObj.value(QLatin1String("@type")).toString(), QLatin1String("QDateTime"));
        QCOMPARE(dtObj.value(QLatin1String("timezone")).toString(), QLatin1String("Europe/Berlin"));
        QCOMPARE(obj.value(QLatin1String("departureDay")).toString(), QLatin1String("2018-03-18"));

        auto obj2 = obj.value(QLatin1String("departureAirport")).toObject();
        QCOMPARE(obj2.value(QLatin1String("@type")).toString(), QLatin1String("Airport"));

        QVERIFY(obj.contains(QLatin1String("departureGate")));
        QCOMPARE(obj.value(QLatin1String("departureGate")).toString(), QLatin1String(""));

        QVERIFY(obj.contains(QLatin1String("airline")));

        qDebug().noquote() << QJsonDocument(obj).toJson();

        // integer values
        FoodEstablishmentReservation res;
        res.setPartySize(2);
        const QString reservationNumber{QStringLiteral("OT123456")};
        res.setReservationNumber(reservationNumber);
        res.setStartTime(QDateTime(QDate(2018, 3, 18), QTime(18, 44, 0), QTimeZone("Europe/Berlin")));
        Person person;
        const QString fullName{QStringLiteral("John")};

        person.setName(fullName);
        res.setUnderName(person);

        array = JsonLdDocument::toJson({res});
        QCOMPARE(array.size(), 1);
        obj = array.at(0).toObject();
        QCOMPARE(obj.value(QLatin1String("partySize")).toInt(), 2);
        QCOMPARE(obj.value(QLatin1String("reservationNumber")).toString(), reservationNumber);
        auto resDtObj = obj.value(QLatin1String("startTime")).toObject();
        QCOMPARE(resDtObj.value(QLatin1String("@value")).toString(), QLatin1String("2018-03-18T18:44:00+01:00"));
        QCOMPARE(resDtObj.value(QLatin1String("@type")).toString(), QLatin1String("QDateTime"));
        QCOMPARE(resDtObj.value(QLatin1String("timezone")).toString(), QLatin1String("Europe/Berlin"));
        qDebug().noquote() << QJsonDocument(obj).toJson();
        auto undernameObj = obj.value(QLatin1String("underName")).toObject();
        QCOMPARE(undernameObj.value(QLatin1String("name")).toString(), QLatin1String("John"));
    }

    void testDeserialization()
    {
        QByteArray b("[{"
            "\"@context\": \"http://schema.org\","
            "\"@type\": \"Flight\","
            "\"departureAirport\": {"
                "\"@type\": \"Airport\","
                "\"iataCode\": \"TXL\","
                "\"name\": \"Berlin Tegel\""
            "},"
            "\"departureTime\": \"2018-03-18T18:44:00+01:00\","
            "\"departureDay\": \"2018-03-17\","
            "\"arrivalTime\": { \"@type\": \"QDateTime\", \"@value\": \"2018-03-18T19:44:00+01:00\", \"timezone\": \"Europe/Berlin\" },"
            "\"departureGate\": \"\","
            "\"flightNumber\": \"1234\""
        "}]");

        auto array = QJsonDocument::fromJson(b).array();
        auto datas = JsonLdDocument::fromJson(array);
        QCOMPARE(datas.size(), 1);

        auto data = datas.at(0);
        QVERIFY(data.canConvert<Flight>());
        Flight flight = data.value<Flight>();
        QCOMPARE(flight.flightNumber(), QLatin1String("1234"));
        QCOMPARE(flight.departureAirport().iataCode(), QLatin1String("TXL"));
        QCOMPARE(flight.departureAirport().name(), QLatin1String("Berlin Tegel"));
        QCOMPARE(flight.departureTime(), QDateTime(QDate(2018, 3, 18), QTime(18, 44, 0), QTimeZone("Europe/Berlin")));
        QCOMPARE(flight.departureDay(), QDate(2018, 3, 17));
        QCOMPARE(flight.arrivalTime(), QDateTime(QDate(2018, 3, 18), QTime(19, 44, 0), QTimeZone("Europe/Berlin")));
        QCOMPARE(flight.arrivalTime().timeSpec(), Qt::TimeZone);
        QCOMPARE(flight.arrivalTime().timeZone(), QTimeZone("Europe/Berlin"));

        QVERIFY(flight.departureGate().isEmpty());
        QVERIFY(!flight.departureGate().isNull());

        // integer values
        b = QByteArray("[{"
            "\"@context\": \"http://schema.org\","
            "\"@type\": \"FoodEstablishmentReservation\","
            "\"partySize\": 42,"
            "\"reservationNumber\": \"0T44542\","
            "\"underName\": {"
            "    \"@type\": \"Person\","
            "    \"name\": \"John Smith\","
            "    \"email\": \"foo@kde.org\""
            "}"

        "}]");

        array = QJsonDocument::fromJson(b).array();
        datas = JsonLdDocument::fromJson(array);
        QCOMPARE(datas.size(), 1);
        data = datas.at(0);
        QVERIFY(data.canConvert<FoodEstablishmentReservation>());
        auto res = data.value<FoodEstablishmentReservation>();
        QCOMPARE(res.partySize(), 42);
        QCOMPARE(res.reservationNumber(), QStringLiteral("0T44542"));
        QCOMPARE(res.underName().value<Person>().name(), QStringLiteral("John Smith"));
        QCOMPARE(res.underName().value<Person>().email(), QStringLiteral("foo@kde.org"));
    }

    void testApply()
    {
        Flight f1;
        f1.setDepartureGate(QLatin1String("38"));
        Airline a1;
        a1.setIataCode(QLatin1String("AB"));
        f1.setAirline(a1);

        Flight f2;
        f2.setDepartureTerminal(QLatin1String("A"));
        Airline a2;
        a2.setName(QLatin1String("Air Berlin"));
        f2.setAirline(a2);

        f1 = JsonLdDocument::apply(f1, f2).value<Flight>();
        QCOMPARE(f1.departureGate(), QLatin1String("38"));
        QCOMPARE(f1.departureTerminal(), QLatin1String("A"));
        QCOMPARE(f1.airline().iataCode(), QLatin1String("AB"));
        QCOMPARE(f1.airline().name(), QLatin1String("Air Berlin"));
    }
};

QTEST_APPLESS_MAIN(JsonLdDocumentTest)

#include "jsonlddocumenttest.moc"
