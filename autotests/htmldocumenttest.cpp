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
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <KItinerary/HtmlDocument>
#include <config-kitinerary.h>

#include <QDebug>
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
#ifdef HAVE_LIBXML2
        std::unique_ptr<HtmlDocument> doc(HtmlDocument::fromData(f.readAll()));
        QVERIFY(doc);
        auto elem = doc->root();
        QVERIFY(!elem.isNull());
        QCOMPARE(elem.name(), QLatin1String("html"));
        QCOMPARE(elem.attribute(QLatin1String("lang")), QLatin1String("de"));
        QVERIFY(elem.nextSibling().isNull());
        QVERIFY(elem.parent().isNull());

        elem = elem.firstChild();
        QVERIFY(!elem.isNull());
        QCOMPARE(elem.name(), QLatin1String("head"));
        elem = elem.nextSibling();
        QVERIFY(!elem.isNull());
        QCOMPARE(elem.name(), QLatin1String("body"));
        QCOMPARE(elem.parent().name(), QLatin1String("html"));

        auto res = doc->eval(QLatin1String("/html"));
        auto nodes = res.toList();
        QCOMPARE(nodes.size(), 1);
        QCOMPARE(nodes.at(0).value<HtmlElement>().name(), QLatin1String("html"));
        nodes = doc->eval(QLatin1String("//body")).toList();
        QCOMPARE(nodes.size(), 1);
        nodes = doc->eval(QLatin1String("//link")).toList();
        QCOMPARE(nodes.size(), 6);

        nodes = doc->eval(QLatin1String("/html/@lang")).toList();
        QCOMPARE(nodes.at(0).value<HtmlElement>().content(), QLatin1String("de"));
        nodes = doc->eval(QLatin1String("//div[@itemtype=\"http://schema.org/FlightReservation\"]")).toList();
        QCOMPARE(nodes.size(), 2);
        elem = nodes.at(0).value<HtmlElement>();
        QCOMPARE(elem.attributes().size(), 2);
        QVERIFY(elem.attributes().contains(QLatin1String("itemscope")));
        QVERIFY(elem.attributes().contains(QLatin1String("itemtype")));
        nodes = elem.eval(QLatin1String("./link")).toList();
        QCOMPARE(nodes.size(), 3);
#endif
    }

    void testContentAccess()
    {
        QFile f(QStringLiteral(SOURCE_DIR "/structureddata/hotel-json-ld-fallback.html"));
        QVERIFY(f.open(QFile::ReadOnly));
#ifdef HAVE_LIBXML2
        std::unique_ptr<HtmlDocument> doc(HtmlDocument::fromData(f.readAll()));
        QVERIFY(doc);
        auto elem = doc->root();
        QVERIFY(!elem.isNull());
        QVERIFY(elem.content().isEmpty());

        elem = elem.firstChild().firstChild().nextSibling();
        QCOMPARE(elem.name(), QLatin1String("script"));
        QCOMPARE(elem.attribute(QLatin1String("type")), QLatin1String("application/ld+json"));
        QCOMPARE(elem.attributes().size(), 1);
        QCOMPARE(elem.attributes().at(0), QLatin1String("type"));
        const auto s = elem.content();
        QVERIFY(s.contains(QLatin1String("checkoutDate")));

        elem = doc->root().firstChild().nextSibling().firstChild();
        QCOMPARE(elem.name(), QLatin1String("p"));
        QCOMPARE(elem.content(), QLatin1String("random content\ncan be invalid"));
#endif
    }

    void testContentProcessing()
    {
        QFile f(QStringLiteral(SOURCE_DIR "/misc/test.html"));
        QVERIFY(f.open(QFile::ReadOnly));
#ifdef HAVE_LIBXML2
        std::unique_ptr<HtmlDocument> doc(HtmlDocument::fromData(f.readAll()));
        QVERIFY(doc);
        auto elem = doc->root();
        QVERIFY(!elem.isNull());
        QVERIFY(elem.content().isEmpty());
        QVERIFY(elem.recursiveContent().contains(QLatin1String("spaces")));

        elem = elem.firstChild().firstChild();
        QCOMPARE(elem.name(), QLatin1String("p"));
        QCOMPARE(elem.content(), QLatin1String("word1\nword2"));

        elem = elem.nextSibling();
        QCOMPARE(elem.name(), QLatin1String("p"));
        QCOMPARE(elem.content(), QLatin1String("lots of spaces"));

        auto elems = doc->eval(QLatin1String("//*[text()[normalize-space(.)='lots of spaces']]")).toList();
        QCOMPARE(elems.size(), 1);
        QCOMPARE(elems.at(0).value<HtmlElement>().name(), QLatin1String("p"));
        elems = doc->eval(QLatin1String("//*[text()='lots of spaces']")).toList();
        QCOMPARE(elems.size(), 0);
#endif
    }
};

QTEST_GUILESS_MAIN(HtmlDocumentTest)

#include "htmldocumenttest.moc"
