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
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include <KItinerary/JsonLdDocument>
#include <KItinerary/Uic9183Parser>

#include <QDebug>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QObject>
#include <QTest>

using namespace KItinerary;

class Uic9183ParserTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testParserValid_data()
    {
        QTest::addColumn<QString>("inFile");
        QTest::addColumn<QString>("refFile");

        QDir dir(QStringLiteral(SOURCE_DIR "/uic918-3/valid"));
        const auto lst = dir.entryList(QStringList(QStringLiteral("*.bin")), QDir::Files | QDir::Readable | QDir::NoSymLinks);
        for (const auto &file : lst) {
            const QString refFile = dir.path() + QLatin1Char('/') + file.left(file.size() - 4) + QStringLiteral(".json");
            if (!QFile::exists(refFile)) {
                qDebug() << "reference file" << refFile << "does not exist, skipping test file" << file;
                continue;
            }
            QTest::newRow(file.toUtf8().constData()) << QString(dir.path() + QLatin1Char('/') +  file) << refFile;
        }
    }

    void testParserValid()
    {
        QFETCH(QString, inFile);
        QFETCH(QString, refFile);

        QFile f(inFile);
        QVERIFY(f.open(QFile::ReadOnly));

        Uic9183Parser p;
        p.parse(f.readAll());
        QVERIFY(p.isValid());

        QFile ref(refFile);
        QVERIFY(ref.open(QFile::ReadOnly));
        const auto refArray = QJsonDocument::fromJson(ref.readAll()).array();
        QVERIFY(!refArray.isEmpty());

        const auto resJson = JsonLdDocument::toJson({QVariant::fromValue(p)});
        if (refArray != resJson) {
            qWarning().noquote() << QJsonDocument(resJson).toJson();
        }
        QCOMPARE(resJson, refArray);
    }

    void testParserInvalid_data()
    {
        QTest::addColumn<QString>("fileName");

        QDir dir(QStringLiteral(SOURCE_DIR "/uic918-3/invalid"));
        const auto lst = dir.entryList(QStringList(QStringLiteral("*.bin")), QDir::Files | QDir::Readable | QDir::NoSymLinks);
        for (const auto &file : lst) {
            QTest::newRow(file.toLatin1().constData()) << QString(dir.path() + QLatin1Char('/') + file);
        }
    }

    void testParserInvalid()
    {
        QFETCH(QString, fileName);
        QFile f(fileName);
        QVERIFY(f.open(QFile::ReadOnly));

        Uic9183Parser p;
        p.parse(f.readAll());
        QVERIFY(!p.isValid());
    }
};

QTEST_APPLESS_MAIN(Uic9183ParserTest)

#include "uic9183parsertest.moc"
