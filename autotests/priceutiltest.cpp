/*
    SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <KItinerary/PriceUtil>

#include <QObject>
#include <QTest>

using namespace KItinerary;

class PriceUtilTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testDecimalCount()
    {
        QCOMPARE(PriceUtil::decimalCount(u"EUR"), 2);
        QCOMPARE(PriceUtil::decimalCount(u"BHD"), 3);
        QCOMPARE(PriceUtil::decimalCount(u"IRR"), 0);
        QCOMPARE(PriceUtil::decimalCount(u"CNY"), 1);
    }
};

QTEST_APPLESS_MAIN(PriceUtilTest)

#include "priceutiltest.moc"
