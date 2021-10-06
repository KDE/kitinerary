/*
  SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <knowledgedb/airportnametokenizer_p.h>

#include <QObject>
#include <QTest>

using namespace KItinerary;

#define s(x) QStringLiteral(x)

class AirportNameTokenizerTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testTokenize_data()
    {
        QTest::addColumn<QString>("text");
        QTest::addColumn<QStringList>("tokens");

        QTest::newRow("empty") << QString() << QStringList();
        QTest::newRow("no token") << s("1a0") << QStringList();
        QTest::newRow("single token") << s("ABC") << QStringList({s("ABC")});
        QTest::newRow("space") << s("abc  def") << QStringList({s("abc"), s("def")});
        QTest::newRow("linebreak") << s("abc\ndef") << QStringList({s("abc"), s("def")});
        QTest::newRow("leading space") << s("     abc def     ") << QStringList({s("abc"), s("def")});
        QTest::newRow("quotes") << s("„abc\" \'def\'") << QStringList({s("abc"), s("def")});
        QTest::newRow("dashes") << s("abc-def–ghi") << QStringList({s("abc"), s("def"), s("ghi")});
        QTest::newRow("short") << s("ab def gh") << QStringList({s("def")});
        QTest::newRow("parenthesis") << s("abc(def)") << QStringList({s("abc"), s("def")});
        QTest::newRow("numbers") << s("01 abc 02 def") << QStringList({s("abc"), s("def")});
        QTest::newRow("ampersand") << s("abc&def") << QStringList({s("abc"), s("def")});
        QTest::newRow("comma") << s("abc, def.") << QStringList({s("abc"), s("def")});

        QTest::newRow("SFO") << s("SFO/SAN FRANCISCO INTERNATIONAL") << QStringList({s("SFO"), s("SAN"), s("FRANCISCO"), s("INTERNATIONAL")});
    }

    void testTokenize()
    {
        QFETCH(QString, text);
        QFETCH(QStringList, tokens);

        AirportNameTokenizer tokenizer(text);
        const auto out = tokenizer.toStringList();
        QCOMPARE(out, tokens);
    }
};

QTEST_APPLESS_MAIN(AirportNameTokenizerTest)

#include "airportnametokenizertest.moc"
