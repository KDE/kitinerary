/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <text/nameoptimizer.cpp>
#include <stringutil.cpp>

#include <KItinerary/Person>

#include <QTest>

using namespace KItinerary;

#define s(x) QStringLiteral(x)

class NameOptimizerTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testOptimizeFirstGiven_data()
    {
        QTest::addColumn<QString>("input");
        QTest::addColumn<QString>("expectedGivenName");
        QTest::addColumn<QString>("expectedFamilyName");

        QTest::newRow("empty") << QString() << s("KONQI THE") << s("DRAGON");
        QTest::newRow("mixed-case") << s("Konqi The Dragon") << s("Konqi The") << s("Dragon");
        QTest::newRow("mixed-case, inverted") << s("Dragon, Konqi The") << s("Konqi The") << s("Dragon");
        QTest::newRow("partial given name") << s("Konqi Dragon") << s("KONQI THE") << s("Dragon");
        QTest::newRow("mixed-case in context") << s("Prenom: Konqi The\nNom: Dragon\nmore text") << s("Konqi The") << s("Dragon");
        QTest::newRow("use in text") << s("Konquering the world with dragonfire with a firedragon") << s("KONQI THE") << s("DRAGON");
        QTest::newRow("diacritics") << s("Konqĩ The Drägon") << s("Konqĩ The") << s("Drägon");
    }

    void testOptimizeFirstGiven()
    {
        QFETCH(QString, input);
        QFETCH(QString, expectedGivenName);
        QFETCH(QString, expectedFamilyName);

        Person p;
        p.setGivenName(s("KONQI THE"));
        p.setFamilyName(s("DRAGON"));

        p = NameOptimizer::optimizeName(input, p);
        QCOMPARE(p.givenName(), expectedGivenName);
        QCOMPARE(p.familyName(), expectedFamilyName);
    }

    void testOptimizeFullName_data()
    {
        QTest::addColumn<QString>("input");
        QTest::addColumn<QString>("expectedName");

        QTest::newRow("empty") << QString() << s("KONQI THE DRAGON");
        QTest::newRow("lower-case") << s("konqi the dragon") << s("KONQI THE DRAGON");
        QTest::newRow("mixed-case") << s("Konqi The Dragon") << s("Konqi The Dragon");
        QTest::newRow("mixed-case, inverted") << s("Dragon, Konqi The") << s("KONQI THE DRAGON");
        QTest::newRow("partial given name") << s("Konqi Dragon") << s("KONQI THE DRAGON");
        QTest::newRow("mixed-case in context") << s("Prenom: Konqi The\nNom: Dragon\nmore text") << s("KONQI THE DRAGON");
        QTest::newRow("use in text") << s("Konquering the world with dragonfire") << s("KONQI THE DRAGON");
        QTest::newRow("diacritics") << s("Könqi The Drägon") << s("Könqi The Drägon");
        QTest::newRow("french-style") << s("Konqi The DRAGON") << s("Konqi The DRAGON");
    }

    void testOptimizeFullName()
    {
        QFETCH(QString, input);
        QFETCH(QString, expectedName);

        Person p;
        p.setName(s("KONQI THE DRAGON"));

        p = NameOptimizer::optimizeName(input, p);
        QCOMPARE(p.name(), expectedName);
    }

    void testPrefixCleaning_data()
    {
        QTest::addColumn<QString>("input");
        QTest::addColumn<QString>("text");
        QTest::addColumn<QString>("expected");

        QTest::newRow("suffix-space-end") << s("KONQI THE DR") << s("Dragon/Konqi The") << s("Konqi The");
        QTest::newRow("suffix-no-space-end") << s("KONQI THEDR") << s("Dragon/Konqi The") << s("Konqi The");
        QTest::newRow("suffix-space") << s("KONQI THE MR") << s("Mr Konqi The Dragon") << s("Konqi The");
        QTest::newRow("suffix-no-space") << s("KONQI THEMR") << s("Mr Konqi The Dragon") << s("Konqi The");
    }
    void testPrefixCleaning()
    {
        QFETCH(QString, input);
        QFETCH(QString, text);
        QFETCH(QString, expected);

        Person p;
        p.setGivenName(input);
        p.setFamilyName(s("DRAGON"));

        p = NameOptimizer::optimizeName(text, p);
        QCOMPARE(p.givenName(), expected);
        QCOMPARE(p.familyName(), QLatin1String("Dragon"));
    }

    void testGivenNameCompletion_data()
    {
        QTest::addColumn<QString>("input");
        QTest::addColumn<QString>("text");
        QTest::addColumn<QString>("expected");

        QTest::newRow("truncated-no-context") << s("SUPERLONGFIRS") << s("Superlongfirstname Dragon") << s("Superlongfirstname");
        QTest::newRow("truncated-context") << s("SUPERLONGFIRS") << s("Passenger name: Superlongfirstname Dragon\nmore text") << s("Superlongfirstname");
        QTest::newRow("slash-no-context") << s("SUPERLONGFIRS") << s("Dragon/Superlongfirstname") << s("Superlongfirstname");
        QTest::newRow("slash-context") << s("SUPERLONGFIRS") << s("Passenger name: Mr Dragon / Superlongfirstname\nmore text") << s("Superlongfirstname");
        QTest::newRow("comma-no-context") << s("SUPERLONGFIRS") << s("Dragon, Superlongfirstname") << s("Superlongfirstname");
        QTest::newRow("comma-context") << s("SUPERLONGFIRS") << s("Passenger name: Mr Dragon, Superlongfirstname\nmore text") << s("Superlongfirstname");
        QTest::newRow("uppper-no-context") << s("SUPERLONGFIRS") << s("SUPERLONGFIRSTNAME DRAGON") << s("SUPERLONGFIRSTNAME");
        QTest::newRow("truncated-context") << s("SUPERLONGFIRS") << s("Passenger name: SUPERLONGFIRSTNAME DRAGON\nmore text") << s("SUPERLONGFIRSTNAME");
    }

    void testGivenNameCompletion()
    {
        QFETCH(QString, input);
        QFETCH(QString, text);
        QFETCH(QString, expected);

        Person p;
        p.setFamilyName(s("DRAGON"));
        p.setGivenName(input);
        p = NameOptimizer::optimizeName(text, p);

        QCOMPARE(p.givenName(), expected);
    }

    void testSpaceExpansion()
    {
        Person p;
        p.setFamilyName(s("THEDRAGON"));
        p.setGivenName(s("KONQI"));
        p = NameOptimizer::optimizeName(s("KONQI THE DRAGON"), p);
        QCOMPARE(p.familyName(), s("THE DRAGON"));
    }
};

QTEST_GUILESS_MAIN(NameOptimizerTest)

#include "nameoptimizertest.moc"
