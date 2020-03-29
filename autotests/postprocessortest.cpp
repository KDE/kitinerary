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

#include "extractorpostprocessor.h"
#include "jsonlddocument.h"

#include <KItinerary/Organization>
#include <KItinerary/Place>

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QObject>
#include <QTest>

using namespace KItinerary;

class PostprocessorTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testPostProc_data()
    {
        QTest::addColumn<QString>("preFile");
        QTest::addColumn<QString>("postFile");

        QDir dir(QStringLiteral(SOURCE_DIR "/postprocessordata"));
        const auto lst = dir.entryList(QStringList(QStringLiteral("*.pre.json")), QDir::Files | QDir::Readable | QDir::NoSymLinks);
        for (const auto &file : lst) {
            const QString refFile = dir.path() + QLatin1Char('/') + file.left(file.size() - 8) + QStringLiteral("post.json");
            if (!QFile::exists(refFile)) {
                qDebug() << "reference file" << refFile << "does not exist, skipping test file" << file;
                continue;
            }
            QTest::newRow(file.toLatin1().constData()) << QString(dir.path() + QLatin1Char('/') +  file) << refFile;
        }
    }

    void testPostProc()
    {
        QFETCH(QString, preFile);
        QFETCH(QString, postFile);

        QFile f(preFile);
        QVERIFY(f.open(QFile::ReadOnly));
        const auto inArray = QJsonDocument::fromJson(f.readAll()).array();
        QVERIFY(!inArray.isEmpty());
        const auto preData = JsonLdDocument::fromJson(inArray);
        QCOMPARE(inArray.size(), preData.size());

        ExtractorPostprocessor postproc;
        postproc.setValidationEnabled(false);
        postproc.setContextDate({QDate(2018, 4, 2), QTime()});
        postproc.process(preData);
        const auto outArray = JsonLdDocument::toJson(postproc.result());
        QCOMPARE(outArray.size(), postproc.result().size());

        QFile ref(postFile);
        QVERIFY(ref.open(QFile::ReadOnly));
        const auto refArray = QJsonDocument::fromJson(ref.readAll()).array();

        if (outArray != refArray) {
            qDebug().noquote() << QJsonDocument(outArray).toJson();
        }
        QCOMPARE(refArray.size(), postproc.result().size());
        QCOMPARE(outArray, refArray);
    }
};

QTEST_APPLESS_MAIN(PostprocessorTest)

#include "postprocessortest.moc"
