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
