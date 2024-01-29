/*
    SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "../lib/era/elbticket.h"

#include <QDebug>
#include <QObject>
#include <QTest>

using namespace KItinerary;

class EraElbTicketTest: public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void parseElbTicket()
    {
        // standard ERA ELB single segment
        auto ticket = ELBTicket::parse("eRIVXXX007123456789121110019ELGAA  01003006034216GBSPXBEBMI09116 91160340130422    XX");
        QVERIFY(ticket);
        QCOMPARE(ticket->pnr(), QLatin1StringView("XXX007"));
        QCOMPARE(ticket->numberAdults(), 1);
        QCOMPARE(ticket->numberChildren(), 0);
        QVERIFY(ticket->segment1().isValid());
        QVERIFY(!ticket->segment2().isValid());
        QCOMPARE(ticket->segment1().arrivalStation(),
                 QLatin1StringView("BEBMI"));

        QDateTime contextDt({2022, 2, 1}, { 8, 0 }, QTimeZone::UTC);
        QCOMPARE(ticket->emissionDate(contextDt), QDate(2013, 1, 6));
        QCOMPARE(ticket->validFromDate(contextDt), QDate(2013, 2, 3));
        QCOMPARE(ticket->validUntilDate(contextDt), QDate(2013, 8, 4));
        QCOMPARE(ticket->segment1().departureDate(contextDt), QDate(2013, 2, 3));

        // single segment with Eurostar signature (?) trailer
        ticket = ELBTicket::parse("eRIVXXX007123456789121110019ELGAA  01003006034216GBSPXBEBMI09116 91160340130422    XXAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA");
        QVERIFY(ticket);
        QVERIFY(ticket->segment1().isValid());
        QVERIFY(!ticket->segment2().isValid());
    }
};

QTEST_GUILESS_MAIN(EraElbTicketTest)

#include "eraelbtickettest.moc"
