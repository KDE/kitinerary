/*
    SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "testhelpers.h"

#include <../src/lib/plist/plistreader.cpp>

#include <QDebug>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QObject>
#include <QTest>

using namespace KItinerary;

class PListReaderTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testPListReader()
    {
        const auto data = Test::readFile(QStringLiteral(SOURCE_DIR "/plist/apple-structured-data-train.plist"));
        PListReader reader(data);
        QVERIFY(reader.isValid());
        QCOMPARE(reader.objectCount(), 228);
        QCOMPARE(reader.rootObjectIndex(), 0);

        QCOMPARE(reader.objectType(0), PListObjectType::Dict);
        QCOMPARE(reader.objectType(1), PListObjectType::String);
        QCOMPARE(reader.object(1).toString(), QLatin1StringView("$version"));
        QCOMPARE(reader.objectType(5), PListObjectType::Int);
        QCOMPARE(reader.object(5).toInt(), 100000);
        QCOMPARE(reader.objectType(6), PListObjectType::String);
        QCOMPARE(reader.object(6).toString(),
                 QLatin1StringView("NSKeyedArchiver"));
    }

    void testUnpackKeyedArchive_data()
    {
        QTest::addColumn<QString>("inFile");
        QTest::addColumn<QString>("refFile");

        QDir dir(QStringLiteral(SOURCE_DIR "/plist"));
        const auto lst = dir.entryList(QStringList(QStringLiteral("*.plist")), QDir::Files | QDir::Readable | QDir::NoSymLinks);
        for (const auto &file : lst) {
            const QString refFile = dir.path() + QLatin1Char('/') + file.left(file.size() - 6) + QStringLiteral(".json");
            if (!QFile::exists(refFile)) {
                qDebug() << "reference file" << refFile << "does not exist, skipping test file" << file;
                continue;
            }
            QTest::newRow(file.toUtf8().constData()) << QString(dir.path() + QLatin1Char('/') +  file) << refFile;
        }
    }

    void testUnpackKeyedArchive()
    {
        QFETCH(QString, inFile);
        QFETCH(QString, refFile);

        const auto data = Test::readFile(inFile);
        QVERIFY(PListReader::maybePList(data));

        PListReader reader(data);
        QVERIFY(reader.isValid());
        const auto unpacked = reader.unpackKeyedArchive().toObject();

        const auto expected = QJsonDocument::fromJson(Test::readFile(refFile)).object();
        if (unpacked != expected) {
            Test::compareJson(refFile, unpacked, expected);
        }
        QCOMPARE(unpacked, expected);
    }
};

QTEST_APPLESS_MAIN(PListReaderTest)

#include "plistreadertest.moc"
