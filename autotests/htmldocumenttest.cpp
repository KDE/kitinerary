/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <KItinerary/HtmlDocument>
#include <config-kitinerary.h>

#include <QFile>
#include <QTest>

using namespace KItinerary;

class HtmlDocumentTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testElementWalking()
    {
        QFile f(QStringLiteral(SOURCE_DIR "/structureddata/os-two-leg-invalid-microdata.html"));
        QVERIFY(f.open(QFile::ReadOnly));
#if HAVE_LIBXML2
        std::unique_ptr<HtmlDocument> doc(HtmlDocument::fromData(f.readAll()));
        QVERIFY(doc);
        auto elem = doc->root();
        QVERIFY(!elem.isNull());
        QCOMPARE(elem.name(), QLatin1StringView("html"));
        QCOMPARE(elem.attribute(QLatin1StringView("lang")),
                 QLatin1StringView("de"));
        QVERIFY(elem.nextSibling().isNull());
        QVERIFY(elem.parent().isNull());

        elem = elem.firstChild();
        QVERIFY(!elem.isNull());
        QCOMPARE(elem.name(), QLatin1StringView("head"));
        elem = elem.nextSibling();
        QVERIFY(!elem.isNull());
        QCOMPARE(elem.name(), QLatin1StringView("body"));
        QCOMPARE(elem.parent().name(), QLatin1StringView("html"));

        auto res = doc->eval(QStringLiteral("/html"));
        auto nodes = res.toList();
        QCOMPARE(nodes.size(), 1);
        QCOMPARE(nodes.at(0).value<HtmlElement>().name(),
                 QLatin1StringView("html"));
        nodes = doc->eval(QStringLiteral("//body")).toList();
        QCOMPARE(nodes.size(), 1);
        nodes = doc->eval(QStringLiteral("//link")).toList();
        QCOMPARE(nodes.size(), 6);

        nodes = doc->eval(QStringLiteral("/html/@lang")).toList();
        QCOMPARE(nodes.at(0).value<HtmlElement>().content(),
                 QLatin1StringView("de"));
        nodes = doc->eval(QStringLiteral("//div[@itemtype=\"http://schema.org/FlightReservation\"]")).toList();
        QCOMPARE(nodes.size(), 2);
        elem = nodes.at(0).value<HtmlElement>();
        QCOMPARE(elem.attributes().size(), 2);
        QVERIFY(elem.attributes().contains(QLatin1StringView("itemscope")));
        QVERIFY(elem.attributes().contains(QLatin1StringView("itemtype")));
        nodes = elem.eval(QStringLiteral("./link")).toList();
        QCOMPARE(nodes.size(), 3);
#endif
    }

    void testContentAccess()
    {
        QFile f(QStringLiteral(SOURCE_DIR "/structureddata/hotel-json-ld-fallback.html"));
        QVERIFY(f.open(QFile::ReadOnly));
#if HAVE_LIBXML2
        std::unique_ptr<HtmlDocument> doc(HtmlDocument::fromData(f.readAll()));
        QVERIFY(doc);
        auto elem = doc->root();
        QVERIFY(!elem.isNull());
        QVERIFY(elem.content().isEmpty());

        elem = elem.firstChild().firstChild().nextSibling();
        QCOMPARE(elem.name(), QLatin1StringView("script"));
        QCOMPARE(elem.attribute(QLatin1StringView("type")),
                 QLatin1StringView("application/ld+json"));
        QCOMPARE(elem.attributes().size(), 1);
        QCOMPARE(elem.attributes().at(0), QLatin1StringView("type"));
        const auto s = elem.content();
        QVERIFY(s.contains(QLatin1StringView("checkoutDate")));

        elem = doc->root().firstChild().nextSibling().firstChild();
        QCOMPARE(elem.name(), QLatin1StringView("p"));
        QCOMPARE(elem.content(),
                 QLatin1StringView("random content\ncan be invalid"));
#endif
    }

    void testContentProcessing()
    {
        QFile f(QStringLiteral(SOURCE_DIR "/misc/test.html"));
        QVERIFY(f.open(QFile::ReadOnly | QFile::Text));
#if HAVE_LIBXML2
        std::unique_ptr<HtmlDocument> doc(HtmlDocument::fromData(f.readAll().replace("<CR>", "\r")));
        QVERIFY(doc);
        auto elem = doc->root();
        QVERIFY(!elem.isNull());
        QVERIFY(elem.content().isEmpty());
        QVERIFY(elem.recursiveContent().contains(QLatin1StringView("spaces")));

        elem = elem.firstChild().firstChild();
        QCOMPARE(elem.name(), QLatin1StringView("p"));
        QCOMPARE(elem.content(), QLatin1StringView("word1\nword2"));
        QCOMPARE(elem.recursiveContent(), QLatin1StringView("word1\nword2"));

        elem = elem.nextSibling();
        QCOMPARE(elem.name(), QLatin1StringView("p"));
        QCOMPARE(elem.content(), QLatin1StringView("lots of spaces"));
        QCOMPARE(elem.recursiveContent(), QLatin1StringView("lots of spaces"));

        auto elems = doc->eval(QStringLiteral("//*[text()[normalize-space(.)='lots of spaces']]")).toList();
        QCOMPARE(elems.size(), 1);
        QCOMPARE(elems.at(0).value<HtmlElement>().name(),
                 QLatin1StringView("p"));
        elems = doc->eval(QStringLiteral("//*[text()='lots of spaces']")).toList();
        QCOMPARE(elems.size(), 0);

        elem = elem.nextSibling();
        QCOMPARE(elem.content(), QString::fromUtf8("인천공항"));
        QCOMPARE(elem.recursiveContent(), QString::fromUtf8("인천공항"));

        elem = elem.nextSibling();
        QCOMPARE(elem.content(), QLatin1StringView("a b"));
        QCOMPARE(elem.recursiveContent(), QLatin1StringView("a b"));

        elem = elem.nextSibling();
        QCOMPARE(elem.content(), QLatin1StringView("a&b"));
        QCOMPARE(elem.recursiveContent(), QLatin1StringView("a&b"));
        elem = elem.nextSibling();
        QCOMPARE(elem.content(), QLatin1StringView("a&b"));
        QCOMPARE(elem.recursiveContent(), QLatin1StringView("a&b"));

        elem = elem.nextSibling();
        QCOMPARE(elem.content(), QLatin1StringView("non breaking"));
        QCOMPARE(elem.recursiveContent(), QLatin1StringView("non breaking"));
        elem = elem.nextSibling();
        QCOMPARE(elem.content(), QLatin1StringView("windows\nline\nbreaks"));
        QCOMPARE(elem.recursiveContent(),
                 QLatin1StringView("windows\nline\nbreaks"));
#endif
    }
};

QTEST_GUILESS_MAIN(HtmlDocumentTest)

#include "htmldocumenttest.moc"
