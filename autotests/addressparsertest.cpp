/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "text/addressparser.cpp"

#include <KItinerary/Place>

#include <QTest>

using namespace KItinerary;

#define s(x) QStringLiteral(x)

class AddressParserTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testPostalCodeExtraction_data()
    {
        QTest::addColumn<QString>("input");
        QTest::addColumn<QString>("city");
        QTest::addColumn<QString>("postalCode");
        QTest::addColumn<QString>("country");

        QTest::newRow("empty") << QString() << QString() << QString() << QString();
        QTest::newRow("no code") << s("PARIS") << s("PARIS") << QString() << QString();
        QTest::newRow("BE-valid") << s("1060 Brussels") << s("Brussels") << s("1060") << s("BE");
        QTest::newRow("BE-invalid") << s("171060 Brussels") << s("171060 Brussels") << QString() << s("BE");
        QTest::newRow("FR-valid") << s("75012 Paris Some Suffix") << s("Paris Some Suffix") << s("75012") << s("FR");
        QTest::newRow("NZ-valid") << s("Palmerston North 4414") << s("Palmerston North") << s("4414") << s("NZ");
        QTest::newRow("PT-valid") << s("1000-205 Lisboa") << s("Lisboa") << s("1000-205") << s("PT");
        QTest::newRow("PT-wrong-country") << s("1000-205 Lisboa") << s("1000-205 Lisboa") << QString() << s("DE");
        QTest::newRow("AR-short") << s("C1420 Buenos Aires") << s("Buenos Aires") << s("C1420") << s("AR");
        QTest::newRow("AR-full") << s("C1420ABC Buenos Aires") << s("Buenos Aires") << s("C1420ABC") << s("AR");
    }

    void testPostalCodeExtraction()
    {
        QFETCH(QString, input);
        QFETCH(QString, city);
        QFETCH(QString, postalCode);
        QFETCH(QString, country);

        PostalAddress a;
        a.setAddressLocality(input);
        a.setAddressCountry(country);

        AddressParser p;
        p.setFallbackCountry(s("GB"));
        p.parse(a);
        const auto out = p.result();
        QCOMPARE(out.addressLocality(), city);
        QCOMPARE(out.postalCode(), postalCode);

        p.parse(out);
        const auto out2 = p.result();
        QCOMPARE(out, out2);
    }
};

QTEST_GUILESS_MAIN(AddressParserTest)

#include "addressparsertest.moc"
