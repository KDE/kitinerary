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
        QEXPECT_FAIL("", "not implemented yet", Continue);
        QVERIFY(v.isValidElement(input.at(0)));

        // TODO test type filtering
    }
};

QTEST_GUILESS_MAIN(ExtractorValidatorTest)

#include "extractorvalidatortest.moc"
