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
#include <KItinerary/Rct2Ticket>
#include <KItinerary/Uic9183Block>
#include <KItinerary/Uic9183TicketLayout>

#include <QDebug>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QObject>
#include <QTest>

using namespace KItinerary;

class Rct2ParserTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testParserValid_data()
    {
        QTest::addColumn<QString>("inFile");
        QTest::addColumn<QString>("refFile");

        QDir dir(QStringLiteral(SOURCE_DIR "/rct2/valid"));
        const auto lst = dir.entryList(QStringList(QStringLiteral("*.rct2")), QDir::Files | QDir::Readable | QDir::NoSymLinks);
        for (const auto &file : lst) {
            const QString refFile = dir.path() + QLatin1Char('/') + file.left(file.size() - 5) + QStringLiteral(".json");
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

        const auto data = f.readAll();
        Uic9183TicketLayout layout(Uic9183Block(data, 0));
        Rct2Ticket rct2(layout);
        rct2.setContextDate({{2018, 12, 19}, {18, 35}});
        QVERIFY(layout.isValid());
        QCOMPARE(layout.type(), QLatin1String("RCT2"));
        QVERIFY(rct2.isValid());

        QFile ref(refFile);
        QVERIFY(ref.open(QFile::ReadOnly));
        const auto refArray = QJsonDocument::fromJson(ref.readAll()).array();
        QVERIFY(!refArray.isEmpty());

        const auto resJson = JsonLdDocument::toJson(QVector<QVariant>({QVariant::fromValue(rct2)}));
        if (refArray != resJson) {
            qWarning().noquote() << QJsonDocument(resJson).toJson();
        }
        QCOMPARE(resJson, refArray);
    }
};

QTEST_APPLESS_MAIN(Rct2ParserTest)

#include "rct2parsertest.moc"
