/*
    Copyright (c) 2018 Volker Krause <vkrause@kde.org>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#include <KItinerary/ExtractorPreprocessor>

#include <QDebug>
#include <QObject>
#include <QTest>

using namespace KItinerary;

class PreprocessorTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testPreProcHtml_data()
    {
        QTest::addColumn<QString>("in");
        QTest::addColumn<QString>("out");

        QTest::newRow("empty") << QString() << QString();
        QTest::newRow("nbsp removal") << QStringLiteral("abc&nbsp;def") << QStringLiteral("abc def");
        QTest::newRow("unknown entity") << QStringLiteral("abc&kde;def") << QStringLiteral("abc&kde;def");
        QTest::newRow("unquoted amp leading") << QStringLiteral("abc&something&nbsp;def") << QStringLiteral("abc&something def");
        QTest::newRow("unquoted amp mid") << QStringLiteral("123&nbsp;abc&def&nbsp;ghi") << QStringLiteral("123 abc&def ghi");
        QTest::newRow("unquoted amp trailing") << QStringLiteral("abc&nbsp;def&ghi") << QStringLiteral("abc def&ghi");
    }

    void testPreProcHtml()
    {
        QFETCH(QString, in);
        QFETCH(QString ,out);

        ExtractorPreprocessor preproc;
        preproc.preprocessHtml(in);
        QCOMPARE(preproc.text(), out);
    }
};

QTEST_APPLESS_MAIN(PreprocessorTest)

#include "preprocessortest.moc"
