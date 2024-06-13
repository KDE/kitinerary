/*
    SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KITINERARY_PRICEFINDER_P_H
#define KITINERARY_PRICEFINDER_P_H

#include <QString>

#include <vector>

namespace KItinerary {

/** Identify price and currency values in text. */
class PriceFinder
{
public:
    explicit PriceFinder();
    ~PriceFinder();

    struct Result {
        // start and end offsets of the price string (possibly including currency information)
        int start = -1;
        int end = -1;
        QString currency; // as ISO code
        double value; // price value

        [[nodiscard]] constexpr inline bool hasResult() const { return start >= 0; }
    };

    /// find all price occurences in @p text
    void findAll(QStringView text, std::vector<Result> &results) const;

    /// find the highest (assumed total) price in @p text
    [[nodiscard]] Result findHighest(QStringView text) const;

    /// returns @c true if the given list of prices uses only a single currency
    [[nodiscard]] bool isSingleCurrency(const std::vector<Result> &results) const;

    /// return the highest (assumed total) price in @p results
    [[nodiscard]] Result highest(std::vector<Result> &results) const;

private:
    enum CurrencyPosition { CurrencyPrefix, CurrencySuffix };
    [[nodiscard]] QString parseCurrency(QStringView s, CurrencyPosition pos) const;
    [[nodiscard]] double parseValue(QStringView s, const QString &isoCode) const;

    struct CurrencyData {
        QString isoCode;
        QString symbol;
    };
    static std::vector<CurrencyData> s_currencyData;
};

}

#endif // KITINERARY_PRICEFINDER_P_H
