/*
    SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KITINERARY_PRICEUTIL_H
#define KITINERARY_PRICEUTIL_H

#include "kitinerary_export.h"

#include <QVariant>

namespace KItinerary {

/** Utilities for dealing with price/currency information of items. */
class PriceUtil
{
public:
    /** Returns @c true if @p item has valid price and currency information. */
    static bool hasPrice(const QVariant &item);

    /** Returns @c true if @p item can have price/currency information. */
    static bool canHavePrice(const QVariant &item);

    /** Returns the price value from @p item. */
    static double price(const QVariant &item);
    /** Returns the currency value from @p item. */
    static QString currency(const QVariant &item);

    /** Sets @p price and @p currency on @p item. */
    static void setPrice(QVariant &item, double price, const QString &currency);

    // TODO add method for computing the total price of a set of items
};

}

#endif
