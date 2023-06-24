/*
  SPDX-FileCopyrightText: 2017 Volker Krause <vkrause@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
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
#include <QProcess>
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
        postproc.setContextDate({QDate(2018, 4, 2), QTime()});
        postproc.process(preData);
        const auto outArray = JsonLdDocument::toJson(postproc.result());
        QCOMPARE(outArray.size(), postproc.result().size());

        QFile ref(postFile);
        QVERIFY(ref.open(QFile::ReadOnly));
        const auto refArray = QJsonDocument::fromJson(ref.readAll()).array();

        if (outArray != refArray) {
            QFile failFile(postFile + QLatin1String(".fail"));
            QVERIFY(failFile.open(QFile::WriteOnly));
            failFile.write(QJsonDocument(outArray).toJson());
            failFile.close();

            QProcess proc;
            proc.setProcessChannelMode(QProcess::ForwardedChannels);
            proc.start(QStringLiteral("diff"), {QStringLiteral("-u"), postFile, failFile.fileName()});
            QVERIFY(proc.waitForFinished());
        }
        QCOMPARE(refArray.size(), postproc.result().size());
        QCOMPARE(outArray, refArray);
    }
};

QTEST_APPLESS_MAIN(PostprocessorTest)

#include "postprocessortest.moc"
