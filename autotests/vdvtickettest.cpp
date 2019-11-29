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
};

QTEST_APPLESS_MAIN(VdvTicketTest)

#include "vdvtickettest.moc"
