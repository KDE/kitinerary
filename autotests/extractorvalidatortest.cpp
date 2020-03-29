/*
    Copyright (C) 2020 Volker Krause <vkrause@kde.org>

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

#include <extractorvalidator.h>

#include <KItinerary/JsonLdDocument>
#include <KItinerary/Reservation>

#include <QDebug>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QTest>

using namespace KItinerary;

class ExtractorValidatorTest : public QObject
{
    Q_OBJECT
private:
    QByteArray readFile(const char *fn) const
    {
        QFile f(QString::fromUtf8(fn));
        f.open(QFile::ReadOnly);
        return f.readAll();
    }
private Q_SLOTS:
    void testValidate()
    {
        ExtractorValidator v;

        auto input = JsonLdDocument::fromJson(QJsonDocument::fromJson(readFile(SOURCE_DIR "/mergedata/cancellation.lhs.json")).array());
        QCOMPARE(input.size(), 1);
        QVERIFY(v.isValidElement(input.at(0)));

        input = JsonLdDocument::fromJson(QJsonDocument::fromJson(readFile(SOURCE_DIR "/mergedata/cancellation.rhs.json")).array());
        QCOMPARE(input.size(), 1);
        QVERIFY(!v.isValidElement(input.at(0)));
        v.setAcceptOnlyCompleteElements(false);
        QVERIFY(v.isValidElement(input.at(0)));

        v.setAcceptOnlyCompleteElements(true);
        v.setAcceptedTypes<FlightReservation, TrainReservation>();
        input = JsonLdDocument::fromJson(QJsonDocument::fromJson(readFile(SOURCE_DIR "/mergedata/cancellation.lhs.json")).array());
        QCOMPARE(input.size(), 1);
        QVERIFY(v.isValidElement(input.at(0)));
        v.setAcceptedTypes<LodgingReservation, TrainReservation>();
        QVERIFY(!v.isValidElement(input.at(0)));
        v.setAcceptedTypes();
        QVERIFY(v.isValidElement(input.at(0)));
    }

    void testFilter_data()
    {
        QTest::addColumn<QString>("preFile");
        QTest::addColumn<QString>("postFile");

        QDir dir(QStringLiteral(SOURCE_DIR "/validationdata"));
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

    void testFilter()
    {
        QFETCH(QString, preFile);
        QFETCH(QString, postFile);

        QFile f(preFile);
        QVERIFY(f.open(QFile::ReadOnly));
        const auto inArray = QJsonDocument::fromJson(f.readAll()).array();
        QVERIFY(!inArray.isEmpty());
        auto preData = JsonLdDocument::fromJson(inArray);
        QCOMPARE(inArray.size(), preData.size());

        ExtractorValidator validator;
        preData.erase(std::remove_if(preData.begin(), preData.end(), [&validator](const auto &elem) {
            return !validator.isValidElement(elem);
        }), preData.end());
        const auto outArray = JsonLdDocument::toJson(preData);

        QFile ref(postFile);
        QVERIFY(ref.open(QFile::ReadOnly));
        const auto refArray = QJsonDocument::fromJson(ref.readAll()).array();

        if (outArray != refArray) {
            qDebug().noquote() << QJsonDocument(outArray).toJson();
        }
        QCOMPARE(refArray.size(), outArray.size());
        QCOMPARE(outArray, refArray);
    }
};

QTEST_GUILESS_MAIN(ExtractorValidatorTest)

#include "extractorvalidatortest.moc"
