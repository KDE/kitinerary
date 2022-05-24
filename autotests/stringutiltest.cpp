/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "../src/lib/stringutil.cpp"

#include <QObject>
#include <QTest>

#define _(x) QStringLiteral(x)

using namespace KItinerary;

class StringUtilTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testNormalize_data()
    {
        QTest::addColumn<QString>("in");
        QTest::addColumn<QString>("out");

        QTest::newRow("empty") << QString() << QString();
        QTest::newRow("normalized") << _("normal") << _("normal");
        QTest::newRow("case-folding") << _("NORMAL") << _("normal");
        QTest::newRow("umlaut") << _("NöRMÄl") << _("normal");
        QTest::newRow("ligature1") << _("ﬁnish") << _("finish");
        QTest::newRow("ligature2") << _("oﬀ") << _("off");
    }

    void testNormalize()
    {
        QFETCH(QString, in);
        QFETCH(QString, out);
        QCOMPARE(StringUtil::normalize(in), out);
    }

    void testPrefixSimilarity()
    {
        QCOMPARE(StringUtil::prefixSimilarity(QString(), QString()), 0.0f);
        QCOMPARE(StringUtil::prefixSimilarity(u"aaa", QString()), 0.0f);
        QCOMPARE(StringUtil::prefixSimilarity(u"aaaa", u"AA"), 0.5f);
        QCOMPARE(StringUtil::prefixSimilarity(u"aba", u"aBa"), 1.0f);
        QCOMPARE(StringUtil::prefixSimilarity(u"ab", u"aa"), 0.5f);
        QCOMPARE(StringUtil::prefixSimilarity(u"ac", u"abbb"), 0.25f);
    }

    void testClean()
    {
        QCOMPARE(StringUtil::clean(QString()), QString());
        QCOMPARE(StringUtil::clean(QStringLiteral("Lech Wa&#322;&#281;sa Airport")), QStringLiteral("Lech Wałęsa Airport"));
        QCOMPARE(StringUtil::clean(QStringLiteral("On-demand services: MobilityData&#39;s GOFS project &amp;amp; going further")),
                 QLatin1String("On-demand services: MobilityData's GOFS project &amp; going further"));

    }
};

QTEST_APPLESS_MAIN(StringUtilTest)

#include "stringutiltest.moc"
