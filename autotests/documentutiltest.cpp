/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <KItinerary/DocumentUtil>
#include <KItinerary/Reservation>

#include <QDebug>
#include <QObject>
#include <QTest>
#include <QVariant>

using namespace KItinerary;

class DocumentUtilTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testDocIdUtils()
    {
        QByteArray docData("%PDF123456");
        const auto docId = DocumentUtil::idForContent(docData);
        QVERIFY(!docId.isEmpty());
        QCOMPARE(docId, DocumentUtil::idForContent(docData));

        FlightReservation flight;
        flight.setSubjectOf(QVariantList{QStringLiteral("someOtherId")});
        QVariant res(flight);

        QVERIFY(DocumentUtil::addDocumentId(res, docId));
        QVERIFY(!DocumentUtil::addDocumentId(res, docId));
        QCOMPARE(res.value<FlightReservation>().subjectOf().size(), 2);
        QVERIFY(res.value<FlightReservation>().subjectOf().contains(docId));

        QVERIFY(DocumentUtil::removeDocumentId(res, docId));
        QVERIFY(!DocumentUtil::removeDocumentId(res, docId));
        QCOMPARE(res.value<FlightReservation>(), flight);
    }
};

QTEST_APPLESS_MAIN(DocumentUtilTest)

#include "documentutiltest.moc"
