/*
    SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "pricefinder_p.h"

#include <QDebug>
#include <QLocale>
#include <QRegularExpression>

#include <cmath>
#include <cstring>

using namespace KItinerary;

std::vector<PriceFinder::CurrencyData> PriceFinder::s_currencyData;

// normalize currency symbols, as e.g. "wide Yen" and "normal Yen" should be considered the same
static QString normalizeSymbol(QStringView str)
{
    QString out;
    out.reserve(str.size());
    for (const auto c : str) {
        if (c.decompositionTag() == QChar::Wide) {
            out.push_back(c.decomposition().at(0));
        } else {
            out.push_back(c);
        }
    }
    return out;
}

static bool isCollidingSymbol(QStringView lhs, QStringView rhs)
{
    return lhs == rhs
        || (lhs.size() == rhs.size() + 1 && lhs.back() == QLatin1Char('.') && lhs.startsWith(rhs))
        || (rhs.size() == lhs.size() + 1 && rhs.back() == QLatin1Char('.') && rhs.startsWith(lhs));
}

// overrides to QLocale data
// ### keep sorted by ISO code
struct {
    const char isoCode[4];
    const char *symbol;
} static constexpr const currency_data_overrides[] = {
    { "BAM", nullptr }, // BAM's symbol is "KM", which collides with distance values on train tickets too often
    { "GBP", "£" }, // FKP, GIP and SHP are practically GPB-equivalent using the pound sign, SSP has it wrongly assigned in QLocale
    { "JPY", "円"}, // the Yen sign is also used by CNY and thus ambigious, but the Japanese Yen symbol works
};

PriceFinder::PriceFinder()
{
    if (!s_currencyData.empty()) {
        return;
    }

    const auto allLocales = QLocale::matchingLocales(QLocale::AnyLanguage, QLocale::AnyScript, QLocale::AnyCountry);
    for (const auto &locale : allLocales) {
        CurrencyData data{locale.currencySymbol(QLocale::CurrencyIsoCode), normalizeSymbol(locale.currencySymbol(QLocale::CurrencySymbol))};
        if (data.isoCode.isEmpty()) {
            continue;
        }

        // single letter symbols tend to be way too trigger-happy
        if (data.symbol.size() == 1 && data.symbol[0].isLetter()) {
            //qDebug() << "Dropping single letter symbol:" << data.symbol << data.isoCode;
            data.symbol.clear();
        }

        s_currencyData.push_back(std::move(data));
    }

    // remove duplicates
    const auto lessThanCurrencyData = [](const auto &lhs, const auto &rhs) {
        return std::tie(lhs.isoCode, lhs.symbol) < std::tie(rhs.isoCode, rhs.symbol);
    };
    std::sort(s_currencyData.begin(), s_currencyData.end(), lessThanCurrencyData);
    const auto compareCurrencyData = [](const auto &lhs, const auto &rhs) {
        return lhs.isoCode == rhs.isoCode && lhs.symbol == rhs.symbol;
    };
    s_currencyData.erase(std::unique(s_currencyData.begin(), s_currencyData.end(), compareCurrencyData), s_currencyData.end());

    // clear ambigious symbols
    for (auto it = s_currencyData.begin(); it != s_currencyData.end(); ++it) {
        if ((*it).symbol.isEmpty()) {
            continue;
        }
        bool collision = false;
        for (auto it2 = std::next(it); it2 != s_currencyData.end(); ++it2) {
            if (!isCollidingSymbol((*it).symbol, (*it2).symbol)) {
                continue;
            }
            (*it2).symbol.clear();
            if (!collision) {
                qDebug() << "Ambigious currency symbol:" << (*it).symbol;
            }
            collision = true;
        }
        if (collision) {
            (*it).symbol.clear();
        }
    }

    // apply our own overrides over QLocale
    for (auto it = s_currencyData.begin(); it != s_currencyData.end(); ++it) {
        const auto it2 = std::lower_bound(std::begin(currency_data_overrides), std::end(currency_data_overrides), (*it).isoCode, [](const auto &lhs, const auto &rhs) {
            return std::strncmp(lhs.isoCode, rhs.toLatin1().constData(), 3) < 0;
        });
        if (it2 == std::end(currency_data_overrides) || std::strncmp((*it2).isoCode, (*it).isoCode.toLatin1().constData(), 3) != 0) {
            continue;
        }
        (*it).symbol = (*it2).symbol ? QString::fromUtf8((*it2).symbol) : QString();
    }
}

PriceFinder::~PriceFinder() = default;

static bool isBoundaryChar(QChar c)
{
    return c != QLatin1Char('-') && (c.isSpace() || c.isPunct() || c.isSymbol());
}

void PriceFinder::findAll(QStringView text, std::vector<Result> &results) const
{
    static QRegularExpression rx(QStringLiteral(R"((?<=\s|[[:punct:]]|^)([^\d\s]{1,4})? *(\d(?:[\d,.  ]*\d)?) *([^\d\s]{1,4})?(?=\s|[[:punct:]]|$))"));

    const auto prevResultSize = results.size();
    qsizetype offset = 0;
    while (true) {
        const auto match = rx.match(text, offset);
        if (!match.hasMatch()) {
            break;
        }
        offset = match.capturedEnd(2);

        const auto leadingCurrency = parseCurrency(match.capturedView(1), CurrencyPrefix);
        const auto trailingCurrency = parseCurrency(match.capturedView(3), CurrencySuffix);
        if ((leadingCurrency.isEmpty() && trailingCurrency.isEmpty()) || (!leadingCurrency.isEmpty() && !trailingCurrency.isEmpty() && leadingCurrency != trailingCurrency)) {
            continue;
        }

        // additional boundary checks not covered by the regular expression
        if (leadingCurrency.isEmpty() && match.capturedStart(2) > 0 && !isBoundaryChar(text[match.capturedStart(2) - 1])) {
            continue;
        }
        if (trailingCurrency.isEmpty() && match.capturedEnd(2) < text.size() - 2 && !isBoundaryChar(text[match.capturedEnd(2)])) {
            continue;
        }

        Result r;
        r.start = leadingCurrency.isEmpty() ? match.capturedStart(2) : match.capturedStart();
        r.end = trailingCurrency.isEmpty() ? match.capturedEnd(2) : match.capturedEnd();
        r.currency = leadingCurrency.isEmpty() ? trailingCurrency : leadingCurrency;

        r.value = parseValue(match.capturedView(2), r.currency);
        if (std::isnan(r.value)) {
            continue;
        }

        results.push_back(std::move(r));
    }

    // check for overlapping results: in those case we have to assume the entire result is invalid
    if (results.size() <= 1 + prevResultSize) {
        return;
    }
    for (auto it = results.begin() + prevResultSize; it != std::prev(results.end()); ++it) {
        if ((*it).end >= (*std::next(it)).start) {
            qDebug() << "overlapping price data, discarding result";
            results.erase(results.begin() + prevResultSize, results.end());
            return;
        }
    }
}

PriceFinder::Result PriceFinder::findHighest(QStringView text) const
{
    std::vector<Result> results;
    findAll(text, results);
    return highest(results);
}

bool PriceFinder::isSingleCurrency(const std::vector<Result> &results) const
{
    if (results.empty()) {
        return false;
    }

    const auto isoCode = results.front().currency;
    return std::all_of(results.begin(), results.end(), [&isoCode](const auto &r) { return r.currency == isoCode; });
}

PriceFinder::Result PriceFinder::highest(const std::vector<Result> &results) const
{
    if (!isSingleCurrency(results)) {
        return {};
    }

    const auto it = std::max_element(results.begin(), results.end(), [](const auto &lhs, const auto &rhs) { return lhs.value < rhs.value; });
    return (*it);
}

static bool equalIgnoreDiacritics(QStringView lhs, QStringView rhs)
{
    if (lhs.size() != rhs.size()) {
        return false;
    }

    for (qsizetype i = 0; i < lhs.size(); ++i) {
        auto l = lhs[i];
        if (l.decompositionTag() == QChar::Canonical) {
            l = l.decomposition().at(0);
        }
        auto r = rhs[i];
        if (r.decompositionTag() == QChar::Canonical) {
            r = r.decomposition().at(0);
        }
        if (l != r) {
            return false;
        }
    }

    return true;
}

QString PriceFinder::parseCurrency(QStringView s, CurrencyPosition pos) const
{
    // trim remaining boundary chars
    if (s.isEmpty()) {
        return {};
    }

    // valid currency ISO code
    auto isoCandidate = s;
    while (!isoCandidate.isEmpty() && isBoundaryChar(isoCandidate.last())) {
        isoCandidate = isoCandidate.left(isoCandidate.size() - 1);
    }
    while (!isoCandidate.isEmpty() && isBoundaryChar(isoCandidate.front())) {
        isoCandidate = isoCandidate.mid(1);
    }
    if (isoCandidate.size() == 3) {
        const auto it = std::lower_bound(s_currencyData.begin(), s_currencyData.end(), isoCandidate, [](const auto &lhs, QStringView rhs) { return lhs.isoCode < rhs; });
        if (it != s_currencyData.end() && (*it).isoCode == isoCandidate) {
            return (*it).isoCode;
        }
    }

    // currency symbol
    const auto symbol = normalizeSymbol(s);
    // exact match: we know there is only ever going to be one (see ctor)
    const auto it = std::find_if(s_currencyData.begin(), s_currencyData.end(), [&symbol](const auto &data) { return data.symbol == symbol; });
    if (it != s_currencyData.end())
        return (*it).isoCode;

    // partial match: needs to be unique
    QString isoCode;
    for (const auto &data : s_currencyData) {
        if (data.symbol.isEmpty()) {
            continue;
        }

        // match disregarding diacritics
        if (equalIgnoreDiacritics(data.symbol, symbol)) {
            if (!isoCode.isEmpty()) {
                return {};
            }
            isoCode = data.isoCode;
        }

        // prefix or suffix match
        if (pos == CurrencyPrefix) {
            if (symbol.size() <= data.symbol.size() || !symbol.endsWith(data.symbol) || !isBoundaryChar(symbol.at(symbol.size() - data.symbol.size() - 1))) {
                continue;
            }
        } else {
            if (symbol.size() <= data.symbol.size() || !symbol.startsWith(data.symbol) || !isBoundaryChar(symbol.at(data.symbol.size()))) {
                continue;
            }
        }
        if (!isoCode.isEmpty()) {
            return {};
        }
        isoCode = data.isoCode;
    }
    return isoCode;
}

// ### keep sorted by ISO code, only needs to contain those != 2
// @see https://en.wikipedia.org/wiki/List_of_circulating_currencies
struct {
    const char isoCode[4];
    const uint8_t decimals;
} static constexpr const currency_decimals_map[] = {
    { "BHD", 3 },
    { "CNY", 1 },
    { "IQD", 3 },
    { "IRR", 0 },
    { "KWD", 3 },
    { "LYD", 3 },
    { "MGA", 1 },
    { "MRU", 1 },
    { "OMR", 3 },
    { "TND", 3 },
    { "VND", 1 },
};

static int decimalsForCurrency(const QString &isoCode)
{
    const auto it = std::lower_bound(std::begin(currency_decimals_map), std::end(currency_decimals_map), isoCode, [](const auto &m, const QString &isoCode) {
        return QLatin1String(m.isoCode, 3) < isoCode;
    });
    if (it != std::end(currency_decimals_map) && QLatin1String((*it).isoCode, 3) == isoCode) {
        return (*it).decimals;
    }
    return 2;
}

double PriceFinder::parseValue(QStringView s, const QString &isoCode) const
{
    if (s.isEmpty() || !s[0].isDigit() || !s[s.size() - 1].isDigit()) {
        return NAN;
    }

    // find potential decimal separator
    QChar decimalSeparator;
    qsizetype decimalSeparatorIndex = -1;
    for (qsizetype i = s.size() - 1; i > 0; --i) {
        if (s[i].isDigit()) {
            continue;
        }
        if (!s[i].isSpace()) {
            decimalSeparator = s[i];
            decimalSeparatorIndex = i;
        }
        break;
    }

    // identify/validate group separators
    QChar groupSeparator;
    qsizetype lastGroupSeparatorIndex = -1;
    for (qsizetype i = 0; i < s.size(); ++i) {
        if (s[i].isDigit()) {
            continue;
        }
        if (lastGroupSeparatorIndex > 0 && i - lastGroupSeparatorIndex != 4) { // separator interval is wrong
            return NAN;
        }
        if (decimalSeparatorIndex > 0 && i == decimalSeparatorIndex) { // found the suspected decimal separator
            break;
        }
        if (!groupSeparator.isNull() && s[i] != groupSeparator) { // inconsistent separators
            return NAN;
        }

        lastGroupSeparatorIndex = i;
        groupSeparator = s[i];
    }

    // we found both and they are the same: has to be the group separator
    if (!decimalSeparator.isNull() && !groupSeparator.isNull() && decimalSeparator == groupSeparator) {
        if ((s.size() - decimalSeparatorIndex) != 4) {
            return NAN;
        }
        decimalSeparator = {};
        decimalSeparatorIndex = -1;
    }

    // we found a decimal separator: verify the number of decimals is consistent with the currency's subdivision
    // see https://en.wikipedia.org/wiki/List_of_circulating_currencies
    if (!decimalSeparator.isNull()) {
        const auto decimalCount = s.size() - decimalSeparatorIndex - 1;
        const auto expectedDecimalCount = decimalsForCurrency(isoCode);

        // subdivision x1000 is ambigious if we don't have a group separator
        if (decimalCount == expectedDecimalCount && decimalCount == 3 && groupSeparator.isNull()) {
            return NAN;
        }

        // if decimal count is 3, assume group separator
        else if (decimalCount != expectedDecimalCount && decimalCount == 3) {
            if (groupSeparator.isNull()) {
                groupSeparator = decimalSeparator;
                decimalSeparator = {};
            } else {
                return NAN;
            }
        }

        else if (decimalCount > expectedDecimalCount) {
            return NAN;
        }
    }

    // strip group separators, replace decimal separator
    auto normalized = s.toString();
    if (!groupSeparator.isNull()) {
        normalized.remove(groupSeparator);
    }
    if (!decimalSeparator.isNull()) {
        normalized.replace(decimalSeparator, QLatin1Char('.'));
    }

    bool ok = false;
    const auto value = normalized.toDouble(&ok);
    if (!ok) {
        return NAN;
    }
    return value;
}
