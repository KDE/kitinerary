/*
    SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <text/pricefinder.cpp>

#include <QTest>

using namespace KItinerary;

#define s(x) QStringLiteral(x)

class PriceFinderTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testFindHighest_data()
    {
        QTest::addColumn<QString>("input");
        QTest::addColumn<int>("start");
        QTest::addColumn<int>("end");
        QTest::addColumn<QString>("currency");
        QTest::addColumn<double>("value");

        QTest::newRow("iso-suffix-1") << s("123.45EUR") << 0 << 9 << s("EUR") << 123.45;
        QTest::newRow("iso-suffix-2") << s("123.45 EUR") << 0 << 10 << s("EUR") << 123.45;
        QTest::newRow("iso-prefix-1") << s("EUR123.45") << 0 << 9 << s("EUR") << 123.45;
        QTest::newRow("iso-prefix-2") << s("EUR 123.45") << 0 << 10 << s("EUR") << 123.45;
        QTest::newRow("symbol-suffix-1") << s("123.45€") << 0 << 7 << s("EUR") << 123.45;
        QTest::newRow("symbol-suffix-2") << s("123.45 €") << 0 << 8 << s("EUR") << 123.45;
        QTest::newRow("symbol-prefix-1") << s("€123.45") << 0 << 7 << s("EUR") << 123.45;
        QTest::newRow("symbol-prefix-2") << s("€ 123.45") << 0 << 8 << s("EUR") << 123.45;

        QTest::newRow("maximum-1") << s("0.50EUR\n1.99EUR\n23.45EUR") << 16 << 24 << s("EUR") << 23.45;
        QTest::newRow("maximum-2") << s("0.50 EUR\n0.50 EUR") << 0 << 8 << s("EUR") << 0.5;

        QTest::newRow("no-decimals-1") << s("7123 CZK") << 0 << 8 << s("CZK") << 7123.0;
        QTest::newRow("no-decimals-2") << s("CZK 7123") << 0 << 8 << s("CZK") << 7123.0;

        QTest::newRow("text-context-1") << s("Total: 123.43USD") << 7 << 16 << s("USD") << 123.43;
        QTest::newRow("text-context-2") << s("Total: 123.43USD\nTaxes: 12.34USD") << 7 << 16 << s("USD") << 123.43;
        QTest::newRow("text-context-3") << s("Total amount\nEUR 244.55\nYour payment") << 13 << 23 << s("EUR") << 244.55;

        QTest::newRow("single-digit-1") << s("1€") << 0 << 2 << s("EUR") << 1.0;
        QTest::newRow("single-digit-2") << s("1 EUR") << 0 << 5 << s("EUR") << 1.0;

        QTest::newRow("group-separator-1") << s("12,345.67 EUR") << 0 << 13 << s("EUR") << 12345.67;
        QTest::newRow("group-separator-2") << s("12 345.67 EUR") << 0 << 13 << s("EUR") << 12345.67;
        QTest::newRow("group-separator-3") << s("9,123,456.78 EUR") << 0 << 16 << s("EUR") << 9123456.78;

        QTest::newRow("comma-format-1") << s("2,34 EUR") << 0 << 8 << s("EUR") << 2.34;
        QTest::newRow("comma-format-2") << s("123,34 EUR") << 0 << 10 << s("EUR") << 123.34;
        QTest::newRow("comma-format-3") << s("4 123,34 EUR") << 0 << 12 << s("EUR") << 4123.34;
        QTest::newRow("comma-format-4") << s("4.123,34 EUR") << 0 << 12 << s("EUR") << 4123.34;

        QTest::newRow("group-separator-no-decimals-1") << s("7,123 CZK") << 0 << 9 << s("CZK") << 7123.0;
        QTest::newRow("group-separator-no-decimals-2") << s("1,237,123 CZK") << 0 << 13 << s("CZK") << 1237123.0;

        QTest::newRow("x1000-subdivision-1") << s("1.234,567 KWD") << 0 << 13 << s("KWD") << 1234.567;
        QTest::newRow("x1000-subdivision-2") << s("1 234 KWD") << 0 << 9 << s("KWD") << 1234.0;

        QTest::newRow("parenthesis") << s("(Price before tax 101.46 EUR)") << 18 << 29 << s("EUR") << 101.46;

        QTest::newRow("double-space-1") << s("123.46  EUR") << 0 << 11 << s("EUR") << 123.46;
        QTest::newRow("double-space-2") << s("EUR  123.46") << 0 << 11 << s("EUR") << 123.46;
        QTest::newRow("double-space-3") << s("ABC1234V 92.00  EUR 1.00  EUR\n") << 10 << 20 << s("EUR") << 92.0;
        QTest::newRow("multi-space") << s("RENTAL CHARGE:                        66.77   EUR UNLIMITED       KM") << 38 << 49 << s("EUR") << 66.77;

        QTest::newRow("no-space-1") << s("Payment collected:GBP375.14/incl VAT |") << 18 << 27 << s("GBP") << 375.14;
        QTest::newRow("no-space-2") << s("Total:GBP375.14") << 6 << 15 << s("GBP") << 375.14;
        QTest::newRow("no-space-3") << s("Total (GBP375.14)") << 6 << 16 << s("GBP") << 375.14;

        QTest::newRow("yen-1") << s("¥6460.00 JPY") << 1 << 12 << s("JPY") << 6460.0;
        QTest::newRow("yen-2") << s("¥6460 円") << 1 << 7 << s("JPY") << 6460.0;

        QTest::newRow("pound-sign-1") << s("£95.90") << 0 << 6 << s("GBP") << 95.90;
        QTest::newRow("pound-sign-2") << s("*£95.90*") << 0 << 7 << s("GBP") << 95.90;
        QTest::newRow("pound-sign-3") << s("￡95.90") << 0 << 6 << s("GBP") << 95.90;

        QTest::newRow("czech-1") << s("Cena 339 Kč") << 5 << 11 << s("CZK") << 339.0;
        QTest::newRow("czech-2") << s("Cena 339 Kc") << 5 << 11 << s("CZK") << 339.0;

        QTest::newRow("non-breaking-space-1") << s("Total price:                         17 704 SEK\n") << 37 << 47 << s("SEK") << 17704.0;
    }

    void testFindHighest()
    {
        QFETCH(QString, input);
        QFETCH(int, start);
        QFETCH(int, end);
        QFETCH(QString, currency);
        QFETCH(double, value);

        PriceFinder finder;
        const auto res = finder.findHighest(input);
        QEXPECT_FAIL("double-space-3", "overlapping price data resolution not implemented", Abort);
        QVERIFY(res.hasResult());
        QCOMPARE(res.start, start);
        QCOMPARE(res.end, end);
        QCOMPARE(res.currency, currency);
        QCOMPARE(res.value, value);
    }

    void testFindHighestNegative_data()
    {
        QTest::addColumn<QString>("input");

        QTest::newRow("empty") << QString();
        QTest::newRow("text") << s("test text");
        QTest::newRow("number") << s("1.23");
        QTest::newRow("currency") << s("EUR");
        QTest::newRow("not-currency-1") << s("123.45 XXX");
        QTest::newRow("not-currency-1") << s("XXX123.45");
        QTest::newRow("not-currency-3") << s("XXXEUR 123.45");

        QTest::newRow("ambigious-symbol-1") << s("$123.45");
        QTest::newRow("ambigious-symbol-2") << s("123.45 kr");
        QTest::newRow("ambigious-symbol-3") << s("123.45 kr.");

        QTest::newRow("group-separator-mix") << s("1,234 567.90 EUR");
        QTest::newRow("wrong-group-separators-1") << s("12,23,45 EUR");
        QTest::newRow("wrong-group-separators-2") << s("12 23 45 EUR");
        QTest::newRow("wrong-group-separators-3") << s("12.23.45 EUR");
        QTest::newRow("wrong-group-separators-4") << s("12,,345.45 EUR");
        QTest::newRow("wrong-group-separators-5") << s("12  345.45 EUR");
        QTest::newRow("wrong-group-separators-6") << s("9,123 456.78 EUR");
        QTest::newRow("wrong-group-separators-7") << s("12,234,45 EUR");
        QTest::newRow("too-many-decimals") << s("12.3454 EUR");

        QTest::newRow("ambigious-currency") << s("USD 123.45 EUR");

        QTest::newRow("x1000-subdivision-1") << s("KWD 1.234");
        QTest::newRow("x1000-subdivision-2") << s("1,234 BHD");
        QTest::newRow("no-subdivision") << s("1.23 IRR");

        QTest::newRow("mixed-currencies-1") << s("123.45 EUR\n234.56 USD");
        QTest::newRow("mixed-currencies-2") << s("Payment collected:EUR375.14/GBP317.47 |");

        QTest::newRow("pnr") << s("S64IHN");
        QTest::newRow("address") << s("3600 S Las Vegas Blvd.");

        QTest::newRow("overlapping") << s("january 2020 € 52,83");

        QTest::newRow("negative") << s("-93.00EUR");

        QTest::newRow("yen-sign-1") << s("¥6460");
        QTest::newRow("yen-sign-2") << s("￥6460");

        QTest::newRow("distance") << s("KM0118");
    }

    void testFindHighestNegative()
    {
        QFETCH(QString, input);
        PriceFinder finder;
        const auto res = finder.findHighest(input);
        QVERIFY(!res.hasResult());
    }
};

QTEST_GUILESS_MAIN(PriceFinderTest)

#include "pricefindertest.moc"
