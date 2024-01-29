/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <KItinerary/ExtractorResult>
#include <KItinerary/Place>

#include <QJsonArray>
#include <QJsonObject>
#include <QTest>

using namespace KItinerary;

#define s(x) QStringLiteral(x)

class ExtractorResultTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testAppend()
    {
        Place p;
        p.setName(s("TEST"));

        ExtractorResult res({p});
        QCOMPARE(res.size(), 1);
        QCOMPARE(res.result().size(), 1);
        QCOMPARE(res.jsonLdResult().size(), 1);

        QJsonObject obj;
        obj.insert(QLatin1StringView("@type"), QLatin1String("Place"));
        obj.insert(QLatin1StringView("name"), QLatin1String("test2"));
        res.append(QJsonArray({obj}));

        QCOMPARE(res.size(), 2);
        QCOMPARE(res.result().size(), 2);
        QCOMPARE(res.jsonLdResult().size(), 2);
    }
};

QTEST_GUILESS_MAIN(ExtractorResultTest)

#include "extractorresulttest.moc"
