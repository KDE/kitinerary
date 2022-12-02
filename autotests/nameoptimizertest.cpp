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
};

QTEST_GUILESS_MAIN(NameOptimizerTest)

#include "nameoptimizertest.moc"
