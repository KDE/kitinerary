/*
    Copyright (c) 2019 Volker Krause <vkrause@kde.org>

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

#include <KItinerary/File>
#include <KItinerary/JsonLdDocument>
#include <KItinerary/Reservation>

#include <KPkPass/Pass>

#include <QByteArray>
#include <QDebug>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QObject>
#include <QTemporaryFile>
#include <QTest>

using namespace KItinerary;

class FileTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testFileIo()
    {
        QTemporaryFile tmp;
        QVERIFY(tmp.open());
        tmp.close();

        File out;
        out.setFileName(tmp.fileName());
        QVERIFY(out.open(File::Write));
        {
            QFile resFile(QLatin1String(SOURCE_DIR "/pkpassdata/swiss.json"));
            QVERIFY(resFile.open(QFile::ReadOnly));
            const auto r = JsonLdDocument::fromJson(QJsonDocument::fromJson(resFile.readAll()).array());
            QCOMPARE(r.size(), 1);
            out.addReservation(r.at(0));

            QFile passFile(QLatin1String(SOURCE_DIR "/pkpassdata/swiss.pkpass"));
            QVERIFY(passFile.open(QFile::ReadOnly));
            const auto passData = passFile.readAll();
            std::unique_ptr<KPkPass::Pass> pass(KPkPass::Pass::fromData(passData));
            QVERIFY(pass);
            out.addPass(pass.get(), passData);
        }
        out.close();

        File in;
        in.setFileName(tmp.fileName());
        QVERIFY(in.open(File::Read));
        QCOMPARE(in.reservations().size(), 1);
        const auto resId = in.reservations().at(0);
        QVERIFY(!resId.isEmpty());
        QVERIFY(!resId.endsWith(QLatin1String(".json")));
        QVERIFY(JsonLd::isA<FlightReservation>(in.reservation(resId)));
        const auto res = in.reservation(resId).value<FlightReservation>();

        QCOMPARE(in.passes().size(), 1);
        const auto passId =  in.passes().at(0);
        QCOMPARE(passId, QLatin1String("pass.booking.swiss.com/MTIzNDU2Nzg5"));
        QVERIFY(!in.passData(passId).isEmpty());
        QCOMPARE(File::passId(res.pkpassPassTypeIdentifier(), res.pkpassSerialNumber()), passId);
    }

    void testMistakes()
    {
        File f;
        f.close();

        QVERIFY(!f.open(File::Read));
        f.setFileName(QLatin1String("foo.itinerary"));
        QVERIFY(!f.open(File::Read));

        QTemporaryFile tmp;
        QVERIFY(tmp.open());
        tmp.close();
        f.setFileName(tmp.fileName());
        QVERIFY(f.open(File::Write));

        QCOMPARE(f.passes(), QVector<QString>());
        QCOMPARE(f.reservations(), QVector<QString>());
        QCOMPARE(f.passData(QStringLiteral("1234")), QByteArray());
        QCOMPARE(f.reservation(QStringLiteral("1234")), QVariant());
    }
};

QTEST_GUILESS_MAIN(FileTest)

#include "filetest.moc"
