/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <KItinerary/ExtractorDocumentNode>

#include <QDebug>
#include <QTest>

using namespace KItinerary;

#define s(x) QStringLiteral(x)

class ExtractorDocumentNodeTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testBasics()
    {
        ExtractorDocumentNode node;
        QVERIFY(node.isNull());
        node = {};
        QVERIFY(node.isNull());

        node.setContent(s("a plain text content"));
        node.setMimeType(s("text/plain"));
        node.setContextDateTime(QDateTime::currentDateTime());
        QVERIFY(node.isNull()); // not properly constructed

        ExtractorDocumentNode child;
        node.appendChild(child);
        QCOMPARE(child.parent().mimeType(), QLatin1String("text/plain"));
        QVERIFY(child.contextDateTime().isValid());
        QCOMPARE(child.contextDateTime(), node.contextDateTime());

        QVERIFY(node.parent().isNull());
        QVERIFY(node.parent().parent().isNull());
    }
};

QTEST_GUILESS_MAIN(ExtractorDocumentNodeTest)

#include "extractordocumentnodetest.moc"
