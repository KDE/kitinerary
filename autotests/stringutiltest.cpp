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

#include "stringutil.h"

#include <QDebug>
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
