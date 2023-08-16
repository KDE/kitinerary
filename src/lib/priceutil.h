/*
    SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KITINERARY_PRICEUTIL_H
#define KITINERARY_PRICEUTIL_H

#include "kitinerary_export.h"

#include <qobjectdefs.h>

class QString;
class QStringView;
class QVariant;

namespace KItinerary {

/** Utilities for dealing with price/currency information of items. */
class KITINERARY_EXPORT PriceUtil
{
    Q_GADGET
public:
    /** Returns @c true if @p item has valid price and currency information. */
    Q_INVOKABLE static bool hasPrice(const QVariant &item);

    /** Returns @c true if @p item can have price/currency information. */
    Q_INVOKABLE static bool canHavePrice(const QVariant &item);

    /** Returns the price value from @p item. */
    Q_INVOKABLE static double price(const QVariant &item);
    /** Returns the currency value from @p item. */
    Q_INVOKABLE static QString currency(const QVariant &item);

    /** Sets @p price and @p currency on @p item. */
    Q_INVOKABLE static void setPrice(QVariant &item, double price, const QString &currency);

    /** Returns the number of decimals to represent the sub-unit of @p currency.
     *  @param currency ISO 4217 currency code
     */
    // TODO this is rather material for KI18nLocaleData
    static int decimalCount(QStringView currency);
    Q_INVOKABLE static int decimalCount(const QString &currency); // QML doesn't support QStringView yet...

    // TODO add method for computing the total price of a set of items
};

}

#endif
