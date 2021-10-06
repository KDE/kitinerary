/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "stringutil.h"

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
    }

    void testNormalize()
    {
        QFETCH(QString, in);
        QFETCH(QString, out);
        QCOMPARE(StringUtil::normalize(in), out);
    }
};

QTEST_APPLESS_MAIN(StringUtilTest)

#include "stringutiltest.moc"
