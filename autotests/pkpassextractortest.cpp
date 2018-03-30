/*
    Copyright (C) 2018 Volker Krause <vkrause@kde.org>

    This program is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This program is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "extractor.h"
#include "extractorengine.h"
#include "extractorrepository.h"
#include "jsonlddocument.h"

#include <KPkPass/BoardingPass>

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QObject>
#include <QTest>

using namespace KItinerary;

class PkPassExtractorTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase()
    {
        // use some exotic locale to ensure the date/time parsing doesn't just work by luck
        QLocale::setDefault(QLocale(QStringLiteral("fr_FR")));
    }

    void testExtractText_data()
    {
        QTest::addColumn<QString>("inputFile");
        QTest::addColumn<QString>("refFile");

        QDir dir(QStringLiteral(SOURCE_DIR "/pkpassdata"));
        const auto lst = dir.entryList(QStringList(QStringLiteral("*.pkpass")), QDir::Files | QDir::Readable | QDir::NoSymLinks);
        for (const auto &file : lst) {
            const QString refFile = dir.path() + QLatin1Char('/') + file.left(file.size() - 7) + QStringLiteral(".json");
            if (!QFile::exists(refFile)) {
                qDebug() << "reference file" << refFile << "does not exist, skipping test file" << file;
                continue;
            }
            QTest::newRow(file.toLatin1().constData()) << QString(dir.path() + QLatin1Char('/') +  file) << refFile;
        }
    }

    void testExtractText()
    {
        QFETCH(QString, inputFile);
        QFETCH(QString, refFile);

        const auto pass = KPkPass::Pass::fromFile(inputFile, this);
        QVERIFY(pass);

        ExtractorRepository repo;
        const auto extractors = repo.extractorsForPass(pass);
        QVERIFY(!extractors.empty());

        ExtractorEngine engine;
        engine.setSenderDate(QDateTime(QDate(2017, 12, 29), QTime(18, 46, 2)));
        engine.setExtractor(extractors.at(0));
        engine.setPass(pass);
        const auto result = JsonLdDocument::toJson(JsonLdDocument::fromJson(engine.extract()));
        QCOMPARE(result.size(), 1);

        QFile ref(refFile);
        QVERIFY(ref.open(QFile::ReadOnly));
        const auto doc = QJsonDocument::fromJson(ref.readAll());
        QVERIFY(doc.isArray());

        if (result != doc.array()) {
            qDebug().noquote() << QJsonDocument(result).toJson();
        }
        QCOMPARE(result, doc.array());
    }
};

QTEST_MAIN(PkPassExtractorTest)

#include "pkpassextractortest.moc"
