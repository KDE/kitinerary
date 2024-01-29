/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>
    SPDX-FileCopyrightText: 2018 Luca Beltrame <lbeltrame@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <KItinerary/BusTrip>
#include <KItinerary/Flight>
#include <KItinerary/Organization>
#include <KItinerary/Place>
#include <KItinerary/Reservation>
#include <KItinerary/TrainTrip>
#include <KItinerary/Ticket>

#include <KItinerary/JsonLdDocument>

#include <QDebug>
#include <QQmlComponent>
#include <QQmlContext>
#include <QQmlEngine>
#include <QObject>
#include <QTest>

using namespace KItinerary;

class DatatypesTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testValueTypeSemantics()
    {
        Airport airport;
        airport.setName(QStringLiteral("Berlin Tegel"));
        {
            auto ap2 = airport;
            QCOMPARE(ap2.name(), QLatin1StringView("Berlin Tegel"));
            ap2.setIataCode(QStringLiteral("TXL"));
            QVERIFY(airport.iataCode().isEmpty());
        }
        Place place = airport; // assignment to base class works, but cannot be reversed
        QCOMPARE(place.name(), QLatin1StringView("Berlin Tegel"));
        place.setName(QStringLiteral("London Heathrow")); // changing a value degenerated to its base class will detach but not slice
        QCOMPARE(place.name(), QLatin1StringView("London Heathrow"));
        QCOMPARE(JsonLdDocument::readProperty(place, "className").toString(),
                 QLatin1StringView("Place")); // className is not polymorphic

        QVERIFY(!airport.geo().isValid());
        QCOMPARE(JsonLdDocument::readProperty(airport, "className").toString(),
                 QLatin1StringView("Airport"));
        QCOMPARE(JsonLdDocument::readProperty(airport, "name").toString(),
                 QLatin1StringView("Berlin Tegel"));

        // detach on base properties must not slice
        FlightReservation res;
        QCOMPARE(JsonLdDocument::readProperty(res, "className").toString(),
                 QLatin1StringView("FlightReservation"));
        res.setAirplaneSeat(QStringLiteral("10E"));
        QCOMPARE(res.airplaneSeat(), QLatin1StringView("10E"));
        auto res2 = res;
        QCOMPARE(res2.airplaneSeat(), QLatin1StringView("10E"));
        res2.setReservationNumber(QStringLiteral("XXX007"));
        QCOMPARE(res2.airplaneSeat(), QLatin1StringView("10E"));

        // changing default-created properties should not leak
        Flight flight;
        QCOMPARE(JsonLdDocument::readProperty(flight, "className").toString(),
                 QLatin1StringView("Flight"));
        QVariant v = flight;
        QVERIFY(v.canConvert<Flight>());
        JsonLdDocument::writeProperty(v, "departureAirport", airport);
        QCOMPARE(v.value<Flight>().departureAirport().name(),
                 QLatin1StringView("Berlin Tegel"));

        // make sure all meta types are generated
        TrainTrip tt;
        QCOMPARE(JsonLdDocument::readProperty(tt, "className").toString(),
                 QLatin1StringView("TrainTrip"));
        BusTrip bus;
        QCOMPARE(JsonLdDocument::readProperty(bus, "className").toString(),
                 QLatin1StringView("BusTrip"));

        Ticket ticket;
        QCOMPARE(
            JsonLdDocument::readProperty(ticket.ticketedSeat(), "className")
                .toString(),
            QLatin1StringView("Seat"));

        Organization org;
        org.setName(QStringLiteral("JR East"));
        org.setEmail(QStringLiteral("nowhere@nowhere.com"));
        org.setTelephone(QStringLiteral("+55-1234-345"));
        org.setUrl(QUrl(QStringLiteral("http://www.jreast.co.jp/e/")));
        QCOMPARE(JsonLdDocument::readProperty(org, "className").toString(),
                 QLatin1StringView("Organization"));
        QCOMPARE(org.name(), QLatin1StringView("JR East"));
        QCOMPARE(org.email(), QLatin1StringView("nowhere@nowhere.com"));
        QCOMPARE(org.telephone(), QLatin1StringView("+55-1234-345"));
        QCOMPARE(org.url(),
                 QUrl(QLatin1StringView("http://www.jreast.co.jp/e/")));

        tt.setProvider(org);
        res.setProvider(org);
        bus.setProvider(org);
        QCOMPARE(tt.provider().name(), QLatin1StringView("JR East"));
        QCOMPARE(tt.provider().email(),
                 QLatin1StringView("nowhere@nowhere.com"));
        QCOMPARE(res.provider().name(), QLatin1StringView("JR East"));
        QCOMPARE(res.provider().email(),
                 QLatin1StringView("nowhere@nowhere.com"));
        QCOMPARE(bus.provider().name(), QLatin1StringView("JR East"));
        QCOMPARE(bus.provider().email(),
                 QLatin1StringView("nowhere@nowhere.com"));

        Airline airline;
        airline.setIataCode(QStringLiteral("LH"));
        flight.setAirline(airline);
        QCOMPARE(flight.airline().iataCode(), QLatin1StringView("LH"));
        {
            const auto flight2 = flight;
            QCOMPARE(flight2.airline().iataCode(), QLatin1StringView("LH"));
            QCOMPARE(JsonLdDocument::readProperty(flight.airline(), "className")
                         .toString(),
                     QLatin1StringView("Airline"));
            QCOMPARE(JsonLdDocument::readProperty(flight.airline(), "iataCode")
                         .toString(),
                     QLatin1StringView("LH"));
        }
    }

    void testQmlCompatibility()
    {
        // one variant and one typed property
        FlightReservation res;
        Flight flight;
        Airport airport;
        airport.setName(QStringLiteral("Berlin Tegel"));
        flight.setDepartureAirport(airport);
        res.setReservationFor(flight);

        QQmlEngine engine;
        engine.rootContext()->setContextProperty(QStringLiteral("_res"), res);
        QQmlComponent component(&engine);
        component.setData("import QtQml 2.2\nQtObject { Component.onCompleted: console.log(_res.reservationFor.departureAirport.name); }", QUrl());
        if (component.status() == QQmlComponent::Error) {
            qWarning() << component.errorString();
        }
        QCOMPARE(component.status(), QQmlComponent::Ready);
        auto obj = component.create();
        QVERIFY(obj);
    }

    void testUpCastHelper()
    {
        Place p;
        FoodEstablishment r;
        Airport a;
        a.setName(QStringLiteral("Berlin Tegel"));
        a.setIataCode(QStringLiteral("TXL"));

        QVERIFY(JsonLd::canConvert<Place>(a));
        QVERIFY(JsonLd::canConvert<Airport>(a));
        QVERIFY(!JsonLd::canConvert<FoodEstablishment>(a));
        QVERIFY(!JsonLd::canConvert<Airport>(p));

        p = JsonLd::convert<Place>(a);
        QCOMPARE(p.name(), QLatin1StringView("Berlin Tegel"));
    }

    void testCompare()
    {
        Place p1;
        Place p2; // base type
        QCOMPARE(p1, p2);
        QCOMPARE(p1, p1);
        p1.setName(QStringLiteral("Berlin"));
        QVERIFY(!(p1 == p2));
        QVERIFY(p1 != p2);
        QCOMPARE(p1, p1);
        p2.setName(QStringLiteral("Berlin"));
        QCOMPARE(p1, p2);

        GeoCoordinates coord1;
        GeoCoordinates coord2; // primitive types
        QCOMPARE(coord1, coord2);
        QCOMPARE(coord1, coord1);
        coord1 = { 52.5, 13.8 };
        QVERIFY(!(coord1 == coord2));

        p1.setGeo(coord1);
        p2.setGeo({52.5, 13.8});
        QCOMPARE(p1, p2);

        Airport a1;
        Airport a2; // polymorphic types
        a1.setIataCode(QStringLiteral("TXL"));
        a2.setIataCode(QStringLiteral("TXL"));
        QCOMPARE(a1, a2);
        a1.setName(QStringLiteral("Berlin Tegel"));
        QVERIFY(a1 != a2);
        a2.setName(QStringLiteral("Berlin Tegel"));
        QCOMPARE(a1, a2);
        QCOMPARE(a1, a1);

        Flight f1;
        f1.setDepartureAirport(a1);
        Flight f2;
        f2.setDepartureAirport(a2);
        QCOMPARE(f1, f2);

        FlightReservation r1;
        r1.setReservationFor(f1);
        FlightReservation r2;
        r2.setReservationFor(f2);
        QCOMPARE(r1, r2);
    }

    void testStrictDateTimeHandling()
    {
        LodgingReservation r;
        r.setCheckinTime(QDateTime({2023, 12, 23}, {15, 1}, QTimeZone::UTC));
        QCOMPARE(r.checkinTime(), QDateTime({2023, 12, 23}, {15, 1}, QTimeZone::UTC));
        QCOMPARE(r.checkinTime().timeSpec(), Qt::UTC);
        r.setCheckinTime(QDateTime({2023, 12, 23}, {16, 1}, QTimeZone::fromSecondsAheadOfUtc(3600)));
        QCOMPARE(r.checkinTime(), QDateTime({2023, 12, 23}, {15, 1}, QTimeZone::UTC));
        QCOMPARE(r.checkinTime().timeSpec(), Qt::OffsetFromUTC);
        r.setCheckinTime(QDateTime({2023, 12, 23}, {16, 1}, QTimeZone("Europe/Brussels")));
        QCOMPARE(r.checkinTime(), QDateTime({2023, 12, 23}, {15, 1}, QTimeZone::UTC));
        QCOMPARE(r.checkinTime().timeSpec(), Qt::TimeZone);
    }
};

QTEST_GUILESS_MAIN(DatatypesTest)

#include "datatypestest.moc"
