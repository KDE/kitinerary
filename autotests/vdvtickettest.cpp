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

#include <vdv/vdvticketparser.h>

#include <QDebug>
#include <QObject>
#include <QTest>

using namespace KItinerary;

class VdvTicketTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testMaybeVdvTicket_data()
    {
        QTest::addColumn<QByteArray>("input");
        QTest::addColumn<bool>("isVdv");

        QTest::newRow("empty") << QByteArray() << false;
        QTest::newRow("null") << QByteArray(352, 0x0) << false;

        QByteArray b(352, 0x0);
        b[0] = (char)0x9E;
        b[1] = (char)0x81;
        b[2] = (char)0x80;
        b[131] = (char)0x9A;
        b[132] = (char)0x05;
        b[133] = 'V';
        b[134] = 'D';
        b[135] = 'V';
        QTest::newRow("valid min length") << b << true;
    }

    void testMaybeVdvTicket()
    {
        QFETCH(QByteArray, input);
        QFETCH(bool, isVdv);
        QCOMPARE(VdvTicketParser::maybeVdvTicket(input), isVdv);
    }

    void testTicket()
    {
        auto data = QByteArray::fromHex("00f569d018f111d017d43b7d68003b8268008532da110000030000000002000992000000000000db0c000000000000000000000000dc0f1117d40000000004b000000000000018f110007a18f13b7974cfd400000017d48a00000105d601000105d602474200000000005644561400");

        VdvTicket ticket(data);
        QCOMPARE(ticket.issuerId(), 6385);
        QCOMPARE(ticket.beginDateTime(), QDateTime({2019, 11, 29}, {13, 0}));
        QCOMPARE(ticket.endDateTime(), QDateTime({2019, 12, 2}, {13, 0}));
        QCOMPARE(ticket.serviceClass(), VdvTicket::SecondClass);
        QCOMPARE(ticket.person(), Person());
        QCOMPARE(ticket.ticketNumber(), QStringLiteral("16083408"));

        data = QByteArray::fromHex("001a4bab1874283e184434ba000134bb18008541da110001000000000003000000000000001a4bdb1502199610144B6174696523447261676F6E00000000dc150000000000000000000000000000000000000000001874110064187434b87128ff7a126918748a0000062e9e0100062e9e007d895644561107");
        ticket = VdvTicket(data);
        QCOMPARE(ticket.issuerId(), 6260);
        QCOMPARE(ticket.beginDateTime(), QDateTime({2016, 5, 26}, {0, 0, 2}));
        QCOMPARE(ticket.endDateTime(), QDateTime({2016, 5, 27}, {3, 0}));
        QCOMPARE(ticket.serviceClass(), VdvTicket::FirstClassUpgrade);
        QCOMPARE(ticket.person().familyName(), QStringLiteral("Dragon"));
        QCOMPARE(ticket.person().givenName(), QStringLiteral("Katie"));
        QCOMPARE(ticket.ticketNumber(), QStringLiteral("1723307"));

        data = QByteArray::fromHex("00f569d018f111d017d43b7d68003b8268008532da110000030000000000000992000000000000db0c00000000004B33654044346Edc0f1117d40000000004b000000000000018f110007a18f13b7974cfd400000017d48a00000105d601000105d602474200000000005644561400");
        ticket = VdvTicket(data);
        QCOMPARE(ticket.serviceClass(), VdvTicket::UnknownClass);
        QCOMPARE(ticket.person().familyName(), QStringLiteral("D"));
        QCOMPARE(ticket.person().givenName(), QStringLiteral("K"));
        QCOMPARE(ticket.ticketNumber(), QStringLiteral("16083408"));
    }
};

QTEST_APPLESS_MAIN(VdvTicketTest)

#include "vdvtickettest.moc"
