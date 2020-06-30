/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <KItinerary/CreativeWork>
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
        QCOMPARE(out.errorString(), QString());
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

            DigitalDocument doc;
            doc.setDescription(QStringLiteral("Ticket"));
            doc.setName(QStringLiteral("../?/../boarding *pass.pdf"));
            doc.setEncodingFormat(QStringLiteral("application/pdf"));
            out.addDocument(QStringLiteral("docid1"), doc, QByteArray("%PDF12345"));

            out.addCustomData(QStringLiteral("org.kde.kitinerary/UnitTest"), QStringLiteral("element1"), QByteArray("hello world"));
            out.addCustomData(QStringLiteral("org.kde.kitinerary/UnitTest"), QStringLiteral("element 2"), QByteArray("hello again"));
            out.addCustomData(QStringLiteral("org.kde.kitinerary/UnitTest2"), QStringLiteral("element1"), QByteArray("something else"));
        }
        out.close();

        File in;
        in.setFileName(tmp.fileName());
        QVERIFY(in.open(File::Read));
        QVERIFY(in.errorString().isEmpty());
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

        const auto docs = in.documents();
        QCOMPARE(docs.size(), 1);
        QCOMPARE(docs.at(0), QLatin1String("docid1"));
        const auto docMeta = in.documentInfo(docs.at(0)).value<DigitalDocument>();
        QCOMPARE(docMeta.name(), QLatin1String("boarding__pass.pdf"));
        QCOMPARE(docMeta.description(), QLatin1String("Ticket"));
        QCOMPARE(docMeta.encodingFormat(), QLatin1String("application/pdf"));
        QCOMPARE(in.documentData(docs.at(0)), QByteArray("%PDF12345"));

        auto customData = in.listCustomData(QStringLiteral("org.kde.kitinerary/UnitTest"));
        QCOMPARE(customData.size(), 2);
        QVERIFY(customData.contains(QLatin1String("element1")));
        QVERIFY(customData.contains(QLatin1String("element 2")));
        QCOMPARE(in.customData(QStringLiteral("org.kde.kitinerary/UnitTest"), QStringLiteral("element1")), QByteArray("hello world"));
        QCOMPARE(in.customData(QStringLiteral("org.kde.kitinerary/UnitTest"), QStringLiteral("element 2")), QByteArray("hello again"));

        customData = in.listCustomData(QStringLiteral("org.kde.kitinerary/UnitTest2"));
        QCOMPARE(customData.size(), 1);
        QVERIFY(customData.contains(QLatin1String("element1")));
        QCOMPARE(in.customData(QStringLiteral("org.kde.kitinerary/UnitTest2"), QStringLiteral("element1")), QByteArray("something else"));
    }

    void testMistakes()
    {
        File f;
        f.close();

        QVERIFY(!f.open(File::Read));
        f.setFileName(QLatin1String("foo.itinerary"));
        QVERIFY(!f.open(File::Read));
        QVERIFY(!f.errorString().isEmpty());

        QTemporaryFile tmp;
        QVERIFY(tmp.open());
        tmp.close();
        f.setFileName(tmp.fileName());
        QVERIFY(f.open(File::Write));
        QVERIFY(f.errorString().isEmpty());

        QCOMPARE(f.passes(), QVector<QString>());
        QCOMPARE(f.reservations(), QVector<QString>());
        QCOMPARE(f.passData(QStringLiteral("1234")), QByteArray());
        QCOMPARE(f.reservation(QStringLiteral("1234")), QVariant());

        f.addDocument(QString(), QVariant(), QByteArray());
        f.addDocument(QStringLiteral("docid"), QVariant(), QByteArray());
        QCOMPARE(f.documents().size(), 0);
        QCOMPARE(f.documentInfo(QStringLiteral("foo")), QVariant());
        QCOMPARE(f.documentData(QStringLiteral("foo")), QByteArray());

        QCOMPARE(f.listCustomData(QStringLiteral("foo")), QVector<QString>());
        QCOMPARE(f.customData(QStringLiteral("a / b"), QStringLiteral("c")), QByteArray());
        QCOMPARE(f.customData(QString(), QString()), QByteArray());
    }

    void testNormalizeDocFileName_data()
    {
        QTest::addColumn<QString>("in");
        QTest::addColumn<QString>("out");

        QTest::newRow("empty") << QString() << QStringLiteral("file");
        QTest::newRow("meta") << QStringLiteral("meta.json") << QStringLiteral("file");
        QTest::newRow("path") << QStringLiteral("a/b/c.pdf") << QStringLiteral("c.pdf");
        QTest::newRow("star") << QStringLiteral("a*b.pdf") << QStringLiteral("a_b.pdf");
        QTest::newRow("questionmark") << QStringLiteral("a?b.pdf") << QStringLiteral("a_b.pdf");
    }

    void testNormalizeDocFileName()
    {
        QFETCH(QString, in);
        QFETCH(QString, out);
        QCOMPARE(File::normalizeDocumentFileName(in), out);
    }
};

QTEST_GUILESS_MAIN(FileTest)

#include "filetest.moc"
