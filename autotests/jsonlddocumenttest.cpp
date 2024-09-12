/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <KItinerary/Flight>
#include <KItinerary/JsonLdDocument>
#include <KItinerary/Organization>
#include <KItinerary/Place>
#include <KItinerary/Reservation>
#include <KItinerary/Person>
#include <KItinerary/Taxi>
#include <KItinerary/RentalCar>
#include <KItinerary/Brand>

#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QObject>
#include <QProcess>
#include <QTest>
#include <QTimeZone>

using namespace KItinerary;

class JsonLdDocumentTest : public QObject
{
    Q_OBJECT
private:
    QByteArray readFile(const QString &fn)
    {
        QFile f(fn);
        f.open(QFile::ReadOnly);
        return f.readAll();
    }

private Q_SLOTS:
    void initTestCase()
    {
        qputenv("TZ", "America/Chicago");
    }

    void testSerialization()
    {
        Flight f;
        f.setFlightNumber(QStringLiteral("1234"));
        f.setDepartureTime(QDateTime(QDate(2018, 3, 18), QTime(18, 44, 0), QTimeZone("Europe/Berlin")));
        f.setDepartureDay(QDate(2018, 3, 18));
        f.setArrivalTime(QDateTime(QDate(2018, 3, 18), QTime(19, 44, 0), QTimeZone::UTC));
        Airport ap;
        ap.setName(QStringLiteral("Berlin Tegel"));
        ap.setIataCode(QStringLiteral("TXL"));
        f.setDepartureAirport(ap);
        f.setDepartureGate(QLatin1StringView(""));
        Airline airline;
        airline.setIataCode(QStringLiteral("LH"));
        f.setAirline(airline);

        auto array =
            JsonLdDocument::toJson(QList<QVariant>({QVariant::fromValue(f)}));
        QCOMPARE(array.size(), 1);
        auto obj = array.at(0).toObject();
        QCOMPARE(obj.value(QLatin1StringView("@context")).toString(),
                 QLatin1StringView("http://schema.org"));
        QCOMPARE(obj.value(QLatin1StringView("@type")).toString(),
                 QLatin1StringView("Flight"));
        QCOMPARE(obj.value(QLatin1StringView("flightNumber")).toString(),
                 QLatin1StringView("1234"));

        QCOMPARE(obj.value(QLatin1StringView("arrivalTime")).toString(),
                 QLatin1StringView("2018-03-18T19:44:00Z"));
        auto dtObj = obj.value(QLatin1StringView("departureTime")).toObject();
        QCOMPARE(dtObj.value(QLatin1StringView("@value")).toString(),
                 QLatin1StringView("2018-03-18T18:44:00+01:00"));
        QCOMPARE(dtObj.value(QLatin1StringView("@type")).toString(),
                 QLatin1StringView("QDateTime"));
        QCOMPARE(dtObj.value(QLatin1StringView("timezone")).toString(),
                 QLatin1StringView("Europe/Berlin"));
        QCOMPARE(obj.value(QLatin1StringView("departureDay")).toString(),
                 QLatin1StringView("2018-03-18"));

        auto obj2 = obj.value(QLatin1StringView("departureAirport")).toObject();
        QCOMPARE(obj2.value(QLatin1StringView("@type")).toString(),
                 QLatin1StringView("Airport"));

        QVERIFY(obj.contains(QLatin1StringView("departureGate")));
        QCOMPARE(obj.value(QLatin1StringView("departureGate")).toString(),
                 QLatin1StringView(""));

        QVERIFY(obj.contains(QLatin1StringView("airline")));

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

        array = JsonLdDocument::toJson(QList<QVariant>({res}));
        QCOMPARE(array.size(), 1);
        obj = array.at(0).toObject();
        QCOMPARE(obj.value(QLatin1StringView("partySize")).toInt(), 2);
        QCOMPARE(obj.value(QLatin1StringView("reservationNumber")).toString(),
                 reservationNumber);
        auto resDtObj = obj.value(QLatin1StringView("startTime")).toObject();
        QCOMPARE(resDtObj.value(QLatin1StringView("@value")).toString(),
                 QLatin1StringView("2018-03-18T18:44:00+01:00"));
        QCOMPARE(resDtObj.value(QLatin1StringView("@type")).toString(),
                 QLatin1StringView("QDateTime"));
        QCOMPARE(resDtObj.value(QLatin1StringView("timezone")).toString(),
                 QLatin1StringView("Europe/Berlin"));
        qDebug().noquote() << QJsonDocument(obj).toJson();
        auto undernameObj =
            obj.value(QLatin1StringView("underName")).toObject();
        QCOMPARE(undernameObj.value(QLatin1StringView("name")).toString(),
                 QLatin1StringView("John"));

        //Rental Car
        RentalCarReservation rentalRes;
        const QString reservationRentalNumber{QStringLiteral("OT1234567")};
        rentalRes.setReservationNumber(reservationRentalNumber);
        Person personRentalCal;
        const QString fullNameRentalCar{QStringLiteral("John2")};

        personRentalCal.setName(fullNameRentalCar);
        rentalRes.setUnderName(personRentalCal);
        rentalRes.setPickupTime(QDateTime(QDate(2018, 3, 18), QTime(18, 44, 0), QTimeZone("Europe/Berlin")));
        rentalRes.setDropoffTime(QDateTime(QDate(2018, 3, 21), QTime(18, 44, 0), QTimeZone("Europe/Berlin")));

        Place placeDropLocation;
        placeDropLocation.setName(QStringLiteral("droplocation"));
        KItinerary::PostalAddress placeDropPostalAddress;
        placeDropPostalAddress.setStreetAddress(QStringLiteral("8 foo bla bla"));
        placeDropPostalAddress.setAddressLocality(QStringLiteral("bli"));
        placeDropLocation.setAddress(placeDropPostalAddress);

        rentalRes.setDropoffLocation(placeDropLocation);

        Place placePickupLocation;
        placePickupLocation.setName(QStringLiteral("pickuplocation"));
        KItinerary::PostalAddress placePickupPostalAddress;
        placePickupPostalAddress.setStreetAddress(QStringLiteral("5 kde foo bla bla"));
        placePickupPostalAddress.setAddressLocality(QStringLiteral("bli2"));
        placePickupLocation.setAddress(placePickupPostalAddress);

        rentalRes.setPickupLocation(placePickupLocation);

        obj = JsonLdDocument::toJson(rentalRes);
        QCOMPARE(obj.value(QLatin1StringView("reservationNumber")).toString(),
                 reservationRentalNumber);

        qDebug().noquote() << QJsonDocument(obj).toJson();
        undernameObj = obj.value(QLatin1StringView("underName")).toObject();
        QCOMPARE(undernameObj.value(QLatin1StringView("name")).toString(),
                 fullNameRentalCar);

        auto pickupTimeObj =
            obj.value(QLatin1StringView("dropoffTime")).toObject();
        QCOMPARE(pickupTimeObj.value(QLatin1StringView("@value")).toString(),
                 QLatin1StringView("2018-03-21T18:44:00+01:00"));
        QCOMPARE(pickupTimeObj.value(QLatin1StringView("@type")).toString(),
                 QLatin1StringView("QDateTime"));
        QCOMPARE(pickupTimeObj.value(QLatin1StringView("timezone")).toString(),
                 QLatin1StringView("Europe/Berlin"));

        auto droptimeObj =
            obj.value(QLatin1StringView("pickupTime")).toObject();
        QCOMPARE(droptimeObj.value(QLatin1StringView("@value")).toString(),
                 QLatin1StringView("2018-03-18T18:44:00+01:00"));
        QCOMPARE(droptimeObj.value(QLatin1StringView("@type")).toString(),
                 QLatin1StringView("QDateTime"));
        QCOMPARE(droptimeObj.value(QLatin1StringView("timezone")).toString(),
                 QLatin1StringView("Europe/Berlin"));
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
        auto flight = data.value<Flight>();
        QCOMPARE(flight.flightNumber(), QLatin1StringView("1234"));
        QCOMPARE(flight.departureAirport().iataCode(),
                 QLatin1StringView("TXL"));
        QCOMPARE(flight.departureAirport().name(),
                 QLatin1StringView("Berlin Tegel"));
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
            "},"
            "\"reservationFor\": {"
            "     \"@type\": \"FoodEstablishment\","
            "     \"name\": \"Wagamama\","
            "     \"address\": {"
            "           \"@type\": \"PostalAddress\","
            "           \"streetAddress\": \"1 Tavistock Street\","
            "           \"addressLocality\": \"London\","
            "           \"addressRegion\": \"Greater London\","
            "           \"postalCode\": \"WC2E 7PG\","
            "           \"addressCountry\": \"United Kingdom\""
            "     }"
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

        const auto foodEstablishment = res.reservationFor().value<FoodEstablishment>();
        const auto address = foodEstablishment.address();
        QCOMPARE(address.addressCountry(), QStringLiteral("United Kingdom"));
        QCOMPARE(address.addressLocality(), QStringLiteral("London"));
        QCOMPARE(address.postalCode(), QStringLiteral("WC2E 7PG"));
        QCOMPARE(address.streetAddress(), QStringLiteral("1 Tavistock Street"));
        QCOMPARE(address.addressRegion(), QStringLiteral("Greater London"));

        //RentalCar
        b = QByteArray("[{"
            "\"@context\": \"http://schema.org\","
            "\"@type\": \"RentalCarReservation\","
            "\"reservationNumber\": \"0T445424\","
            "\"underName\": {"
            "    \"@type\": \"Person\","
            "    \"name\": \"John Smith\","
            "    \"email\": \"foo@kde.org\""
            "},"
            "\"pickupLocation\": {"
            "    \"@type\": \"Place\","
            "    \"address\": {"
            "       \"@type\": \"PostalAddress\","
            "       \"addressLocality\": \"bli2\","
            "       \"streetAddress\": \"5 kde foo bla bla\""
            "},"
            "\"name\": \"pickuplocation\""
            "},"
            "\"dropoffLocation\": {"
            "    \"@type\": \"Place\","
            "    \"address\": {"
            "       \"@type\": \"PostalAddress\","
            "       \"addressLocality\": \"bli3\","
            "       \"streetAddress\": \"7 kde foo bla bla\""
            "},"
            "\"name\": \"droplocation\""
            "},"
            "\"reservationFor\": {"
            "    \"@type\": \"RentalCar\","
            "    \"name\": \"Economy Class Car\","
            "    \"model\": \"Civic\","
            "    \"brand\": {"
            "       \"@type\": \"Brand\","
            "       \"name\": \"Honda\""
            "    },"
            "    \"rentalCompany\": {"
            "          \"@type\": \"Organization\","
            "          \"name\": \"Hertz\""
            "    }"
            "},"
            "\"pickupTime\": \"2018-03-18T18:44:00+01:00\","
            "\"dropoffTime\": \"2018-03-21T18:44:00+01:00\""
        "}]");

        array = QJsonDocument::fromJson(b).array();
        datas = JsonLdDocument::fromJson(array);
        QCOMPARE(datas.size(), 1);
        data = datas.at(0);
        QVERIFY(data.canConvert<RentalCarReservation>());
        auto resRentCar = data.value<RentalCarReservation>();
        QCOMPARE(resRentCar.reservationNumber(), QStringLiteral("0T445424"));
        QCOMPARE(resRentCar.underName().value<Person>().name(), QStringLiteral("John Smith"));
        QCOMPARE(resRentCar.underName().value<Person>().email(), QStringLiteral("foo@kde.org"));
        QCOMPARE(resRentCar.pickupTime(), QDateTime(QDate(2018, 3, 18), QTime(18, 44, 0), QTimeZone("Europe/Berlin")));
        QCOMPARE(resRentCar.dropoffTime(), QDateTime(QDate(2018, 3, 21), QTime(18, 44, 0), QTimeZone("Europe/Berlin")));
        const auto dropoffLocation = resRentCar.dropoffLocation();
        QCOMPARE(dropoffLocation.name(), QStringLiteral("droplocation"));
        const auto dropOffLocationAddress = dropoffLocation.address();
        QCOMPARE(dropOffLocationAddress.streetAddress(), QStringLiteral("7 kde foo bla bla"));
        QCOMPARE(dropOffLocationAddress.addressLocality(), QStringLiteral("bli3"));

        const auto pickupLocation = resRentCar.pickupLocation();
        QCOMPARE(pickupLocation.name(), QStringLiteral("pickuplocation"));
        const auto pickupLocationAddress = pickupLocation.address();
        QCOMPARE(pickupLocationAddress.streetAddress(), QStringLiteral("5 kde foo bla bla"));
        QCOMPARE(pickupLocationAddress.addressLocality(), QStringLiteral("bli2"));

        const auto reservationForRentalCar = resRentCar.reservationFor().value<RentalCar>();
        QCOMPARE(reservationForRentalCar.name(), QStringLiteral("Economy Class Car"));
        QCOMPARE(reservationForRentalCar.model(), QStringLiteral("Civic"));
        const auto brand = reservationForRentalCar.brand();
        QCOMPARE(brand.name(), QStringLiteral("Honda"));

        const auto rentalCarOrganization = reservationForRentalCar.rentalCompany();
        QCOMPARE(rentalCarOrganization.name(), QStringLiteral("Hertz"));

        //Taxi Reservation
        b = QByteArray("[{"
            "\"@context\": \"http://schema.org\","
            "\"@type\": \"TaxiReservation\","
            "\"reservationId\": \"0T445424\","
            "\"underName\": {"
            "    \"@type\": \"Person\","
            "    \"name\": \"John Smith2\","
            "    \"email\": \"foo@kde.org\""
            "},"
            "\"pickupLocation\": {"
            "    \"@type\": \"Place\","
            "    \"address\": {"
            "       \"@type\": \"PostalAddress\","
            "       \"addressLocality\": \"bli2\","
            "       \"streetAddress\": \"5 kde foo bla bla\""
            "     }"
            "},"
            "\"reservationFor\": {"
            "    \"@type\": \"Taxi\","
            "    \"provider\": {"
            "       \"@type\": \"Organization\","
            "       \"name\": \"Checker Cab\""
            "    }"
            "},"
            "\"pickupTime\": \"2018-03-18T18:44:00+01:00\""
        "}]");
        array = QJsonDocument::fromJson(b).array();
        datas = JsonLdDocument::fromJson(array);
        QCOMPARE(datas.size(), 1);
        data = datas.at(0);
        QVERIFY(data.canConvert<TaxiReservation>());
        auto resTaxi = data.value<TaxiReservation>();
        QCOMPARE(resTaxi.reservationNumber(), QStringLiteral("0T445424"));
        QCOMPARE(resTaxi.underName().value<Person>().name(), QStringLiteral("John Smith2"));
        QCOMPARE(resTaxi.underName().value<Person>().email(), QStringLiteral("foo@kde.org"));

        const auto pickUpLocationTaxi = resTaxi.pickupLocation();
        const auto pickupLocationAddressTaxi = pickUpLocationTaxi.address();
        QCOMPARE(pickupLocationAddressTaxi.streetAddress(), QStringLiteral("5 kde foo bla bla"));
        QCOMPARE(pickupLocationAddressTaxi.addressLocality(), QStringLiteral("bli2"));

        QCOMPARE(resTaxi.pickupTime(), QDateTime(QDate(2018, 3, 18), QTime(18, 44, 0), QTimeZone("Europe/Berlin")));
        QVERIFY(resTaxi.reservationFor().canConvert<Taxi>());
    }

    void testApply()
    {
        Flight f1;
        f1.setDepartureGate(QStringLiteral("38"));
        Airline a1;
        a1.setIataCode(QStringLiteral("AB"));
        f1.setAirline(a1);

        Flight f2;
        f2.setDepartureTerminal(QStringLiteral("A"));
        Airline a2;
        a2.setName(QStringLiteral("Air Berlin"));
        f2.setAirline(a2);

        f1 = JsonLdDocument::apply(f1, f2).value<Flight>();
        QCOMPARE(f1.departureGate(), QLatin1StringView("38"));
        QCOMPARE(f1.departureTerminal(), QLatin1StringView("A"));
        QCOMPARE(f1.airline().iataCode(), QLatin1StringView("AB"));
        QCOMPARE(f1.airline().name(), QLatin1StringView("Air Berlin"));
    }

    void testDateTimeParsing_data()
    {
        QTest::addColumn<QByteArray>("dtStr");
        QTest::addColumn<QDateTime>("result");

        QTest::newRow("iso") << QByteArray("2018-08-15T22:00:00+02:00") << QDateTime({2018, 8, 15}, {22 ,00}, QTimeZone::fromSecondsAheadOfUtc(7200));
        QTest::newRow("weird LH format") << QByteArray("20200826T200000Z") << QDateTime({2020, 8, 26}, {20 ,00}, QTimeZone::UTC);
    }

    void testDateTimeParsing()
    {
        QFETCH(QByteArray, dtStr);
        QFETCH(QDateTime, result);

        const auto b = QByteArray("[{"
            "\"@context\": \"http://schema.org\","
            "\"@type\": \"Flight\","
            "\"departureTime\": \"" + dtStr + "\""
        "}]");
        const auto array = QJsonDocument::fromJson(b).array();
        const auto data = JsonLdDocument::fromJson(array);
        QCOMPARE(data.size(), 1);
        const auto flight = data[0].value<Flight>();
        QCOMPARE(flight.departureTime(), result);
        QCOMPARE(flight.departureTime().timeSpec(), result.timeSpec());
    }

    void testNormalize_data()
    {
        QTest::addColumn<QString>("inFile");
        QTest::addColumn<QString>("refFile");

        QDir dir(QStringLiteral(SOURCE_DIR "/jsonlddata"));
        const auto lst = dir.entryList(QStringList(QStringLiteral("*.in.json")), QDir::Files | QDir::Readable | QDir::NoSymLinks);
        for (const auto &file : lst) {
            const QString refFile = dir.path() + QLatin1Char('/') + file.left(file.size() - 7) + QStringLiteral("out.json");
            if (!QFile::exists(refFile)) {
                qDebug() << "reference file" << refFile << "does not exist, skipping test file" << file;
                continue;
            }
            QTest::newRow(file.toLatin1().constData()) << QString(dir.path() + QLatin1Char('/') +  file) << refFile;
        }
    }

    void testNormalize()
    {
        QFETCH(QString, inFile);
        QFETCH(QString, refFile);

        const auto inJson = QJsonDocument::fromJson(readFile(inFile)).array();
        QVERIFY(!inJson.isEmpty());
        const auto normalizedJson = JsonLdDocument::toJson(JsonLdDocument::fromJson(inJson));
        QVERIFY(!normalizedJson.isEmpty());
        const auto refJson = QJsonDocument::fromJson(readFile(refFile)).array();
        QVERIFY(!refJson.isEmpty());

        qDebug() << normalizedJson;
        if (normalizedJson != refJson) {
          QFile f(refFile + QLatin1StringView(".fail"));
          QVERIFY(f.open(QFile::WriteOnly));
          f.write(QJsonDocument(normalizedJson).toJson());
          f.close();

          QProcess proc;
          proc.setProcessChannelMode(QProcess::ForwardedChannels);
          proc.start(QStringLiteral("diff"),
                     {QStringLiteral("-u"), refFile, f.fileName()});
          QVERIFY(proc.waitForFinished());
        }
        QCOMPARE(normalizedJson, refJson);
    }
};

QTEST_APPLESS_MAIN(JsonLdDocumentTest)

#include "jsonlddocumenttest.moc"
