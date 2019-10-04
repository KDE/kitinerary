/*
    Copyright (C) 2019 Volker Krause <vkrause@kde.org>

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

#include <KItinerary/Extractor>
#include <KItinerary/ExtractorRepository>

#include <QDebug>
#include <QObject>
#include <QTest>
#include <QVariant>

using namespace KItinerary;

class ExtractorRepositoryTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testReload()
    {
        ExtractorRepository repo;
        const auto count = repo.allExtractors().size();
        QVERIFY(count > 0);
        repo.reload();
        QCOMPARE(repo.allExtractors().size(), count);
    }

    void testApplyFilter()
    {
        ExtractorRepository repo;

        std::vector<Extractor> extractors;
        repo.extractorsForBarcode(QStringLiteral("i0CVxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxX"), extractors);
        QCOMPARE(extractors.size(), 1);
        QVERIFY(extractors[0].name().startsWith(QLatin1String("sncf")));

        extractors.clear();
        repo.extractorsForContent(QStringLiteral("PNR:1234567890,TRAIN:12345,DOJ:dd-mm-yyyy,TIME:hh:mm,SL,A TO B,John Doe+2,S7 49 S7 52 S7 55,FARE:140,SC:10+PG CHGS."), extractors);
        QCOMPARE(extractors.size(), 1);
        QVERIFY(extractors[0].name().startsWith(QLatin1String("irctc")));
    }
};

QTEST_APPLESS_MAIN(ExtractorRepositoryTest)

#include "extractorrepositorytest.moc"
