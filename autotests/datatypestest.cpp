/*
    Copyright (c) 2018 Volker Krause <vkrause@kde.org>
    Copyright (c) 2018 Luca Beltrame <lbeltrame@kde.org>

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
        airport.setName(QLatin1String("Berlin Tegel"));
        {
            auto ap2 = airport;
            QCOMPARE(ap2.name(), QLatin1String("Berlin Tegel"));
            ap2.setIataCode(QLatin1String("TXL"));
            QVERIFY(airport.iataCode().isEmpty());
        }
        Place place = airport; // assignment to base class works, but cannot be reversed
        QCOMPARE(place.name(), QLatin1String("Berlin Tegel"));
        place.setName(QLatin1String("London Heathrow")); // changing a value degenerated to its base class will detach but not slice
        QCOMPARE(place.name(), QLatin1String("London Heathrow"));
        QCOMPARE(JsonLdDocument::readProperty(QVariant::fromValue(place), "className").toString(), QLatin1String("Place")); // className is not polymorphic

        QVERIFY(!airport.geo().isValid());
        QCOMPARE(JsonLdDocument::readProperty(QVariant::fromValue(airport), "className").toString(), QLatin1String("Airport"));
        QCOMPARE(JsonLdDocument::readProperty(QVariant::fromValue(airport), "name").toString(), QLatin1String("Berlin Tegel"));

        // detach on base properties must not slice
        FlightReservation res;
        QCOMPARE(JsonLdDocument::readProperty(QVariant::fromValue(res), "className").toString(), QLatin1String("FlightReservation"));
        res.setAirplaneSeat(QLatin1String("10E"));
        QCOMPARE(res.airplaneSeat(), QLatin1String("10E"));
        auto res2 = res;
        QCOMPARE(res2.airplaneSeat(), QLatin1String("10E"));
        res2.setReservationNumber(QLatin1String("XXX007"));
        QCOMPARE(res2.airplaneSeat(), QLatin1String("10E"));

        // changing default-created properties should not leak
        Flight flight;
        QCOMPARE(JsonLdDocument::readProperty(QVariant::fromValue(flight), "className").toString(), QLatin1String("Flight"));
        auto v = QVariant::fromValue(flight);
        QVERIFY(v.canConvert<Flight>());
        JsonLdDocument::writeProperty(v, "departureAirport", QVariant::fromValue(airport));
        QCOMPARE(v.value<Flight>().departureAirport().name(), QLatin1String("Berlin Tegel"));

        // make sure all meta types are generated
        TrainTrip tt;
        QCOMPARE(JsonLdDocument::readProperty(QVariant::fromValue(tt), "className").toString(), QLatin1String("TrainTrip"));
        BusTrip bus;
        QCOMPARE(JsonLdDocument::readProperty(QVariant::fromValue(bus), "className").toString(), QLatin1String("BusTrip"));

        Ticket ticket;
        QCOMPARE(JsonLdDocument::readProperty(QVariant::fromValue(ticket.ticketedSeat()), "className").toString(), QLatin1String("Seat"));

        Organization org;
        org.setName(QLatin1String("JR East"));
        org.setEmail(QLatin1String("nowhere@nowhere.com"));
        org.setTelephone(QLatin1String("+55-1234-345"));
        org.setUrl(QUrl(QLatin1String("http://www.jreast.co.jp/e/")));
        QCOMPARE(JsonLdDocument::readProperty(QVariant::fromValue(org), "className").toString(), QLatin1String("Organization"));
        QCOMPARE(org.name(), QLatin1String("JR East"));
        QCOMPARE(org.email(), QLatin1String("nowhere@nowhere.com"));
        QCOMPARE(org.telephone(), QLatin1String("+55-1234-345"));
        QCOMPARE(org.url(), QUrl(QLatin1String("http://www.jreast.co.jp/e/")));

        tt.setProvider(org);
        flight.setProvider(org);
        res.setProvider(org);
        bus.setProvider(org);
        QCOMPARE(tt.provider().name(), QLatin1String("JR East"));
        QCOMPARE(tt.provider().email(), QLatin1String("nowhere@nowhere.com"));
        QCOMPARE(flight.provider().name(), QLatin1String("JR East"));
        QCOMPARE(flight.provider().email(), QLatin1String("nowhere@nowhere.com"));
        QCOMPARE(res.provider().name(), QLatin1String("JR East"));
        QCOMPARE(res.provider().email(), QLatin1String("nowhere@nowhere.com"));
        QCOMPARE(bus.provider().name(), QLatin1String("JR East"));
        QCOMPARE(bus.provider().email(), QLatin1String("nowhere@nowhere.com"));

        Airline airline;
        airline.setIataCode(QLatin1String("LH"));
        flight.setAirline(airline);
        QCOMPARE(flight.airline().iataCode(), QLatin1String("LH"));
        {
            const auto flight2 = flight;
            QCOMPARE(flight2.airline().iataCode(), QLatin1String("LH"));
            QCOMPARE(JsonLdDocument::readProperty(QVariant::fromValue(flight.airline()), "className").toString(), QLatin1String("Airline"));
            QCOMPARE(JsonLdDocument::readProperty(QVariant::fromValue(flight.airline()), "iataCode").toString(), QLatin1String("LH"));
        }
    }

    void testQmlCompatibility()
    {
        // one variant and one typed property
        FlightReservation res;
        Flight flight;
        Airport airport;
        airport.setName(QLatin1String("Berlin Tegel"));
        flight.setDepartureAirport(airport);
        res.setReservationFor(QVariant::fromValue(flight));

        QQmlEngine engine;
        engine.rootContext()->setContextProperty(QLatin1String("_res"), QVariant::fromValue(res));
        QQmlComponent component(&engine);
        component.setData("import QtQml 2.2\nQtObject { Component.onCompleted: console.log(_res.reservationFor.departureAirport.name); }", QUrl());
        if (component.status() == QQmlComponent::Error) {
            qWarning() << component.errorString();
        }
        QCOMPARE(component.status(), QQmlComponent::Ready);
        auto obj = component.create();
        QVERIFY(obj);
    }
};

QTEST_MAIN(DatatypesTest)

#include "datatypestest.moc"

