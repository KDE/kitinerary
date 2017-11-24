/*
  Copyright (c) 2017 Volker Krause <vkrause@kde.org>

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

#include "../semantic/structureddataextractor.cpp"

#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QObject>
#include <QTest>

class StructuredDataExtractorTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testExtract_data()
    {
        QTest::addColumn<QString>("inputFile");
        QTest::addColumn<QString>("jsonFile");

        QDir dir(QStringLiteral(SOURCE_DIR "/structureddata"));
        const auto lst = dir.entryList(QStringList(QStringLiteral("*.html")), QDir::Files | QDir::Readable | QDir::NoSymLinks);
        for (const auto &file : lst) {
            const auto refFile = dir.path() + QLatin1Char('/') + file.left(file.size() - 5) + QStringLiteral(".json");
            if (!QFile::exists(refFile)) {
                qDebug() << "reference file" << refFile << "does not exist, skipping test file" << file;
                continue;
            }
            QTest::newRow(file.toLatin1()) << QString(dir.path() + QLatin1Char('/') +  file) << refFile;
        }
    }

    void testExtract()
    {
        QFETCH(QString, inputFile);
        QFETCH(QString, jsonFile);

        StructuredDataExtractor extractor;
        QFile f(inputFile);
        QVERIFY(f.open(QFile::ReadOnly));
        extractor.parse(QString::fromUtf8(f.readAll()));

        QFile ref(jsonFile);
        QVERIFY(ref.open(QFile::ReadOnly));
        const auto doc = QJsonDocument::fromJson(ref.readAll());
        QVERIFY(doc.isArray());

        if (extractor.data() != doc.array()) {
            qDebug().noquote() << QJsonDocument(extractor.data()).toJson();
        }
        QCOMPARE(extractor.data(), doc.array());
    }
};

QTEST_APPLESS_MAIN(StructuredDataExtractorTest)

#include "structureddataextractortest.moc"
