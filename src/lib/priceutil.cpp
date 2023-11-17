/*
    SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "priceutil.h"

#include <KItinerary/JsonLdDocument>
#include <KItinerary/Reservation>
#include <KItinerary/Ticket>

#include <cmath>

using namespace KItinerary;

bool PriceUtil::hasPrice(const QVariant &item)
{
    if (!PriceUtil::canHavePrice(item)) {
        return false;
    }

    if (!std::isnan(JsonLdDocument::readProperty(item, "totalPrice").toDouble()) && !JsonLdDocument::readProperty(item, "priceCurrency").isNull()) {
        return true;
    }

    if (JsonLd::canConvert<Reservation>(item)) {
        return PriceUtil::hasPrice(JsonLd::convert<Reservation>(item).reservedTicket());
    }

    return false;
}

bool PriceUtil::canHavePrice(const QVariant &item)
{
    return JsonLd::isA<Ticket>(item) || (JsonLd::canConvert<Reservation>(item) && !JsonLd::isA<FoodEstablishmentReservation>(item));
}

double PriceUtil::price(const QVariant &item)
{
    if (JsonLd::canConvert<Reservation>(item)) {
        const auto res = JsonLd::convert<Reservation>(item);
        return  !std::isnan(res.totalPrice()) ? res.totalPrice() : PriceUtil::price(res.reservedTicket());
    }

    if (JsonLd::isA<Ticket>(item)) {
        return item.value<Ticket>().totalPrice();
    }

    return NAN;
}

QString PriceUtil::currency(const QVariant &item)
{
    if (JsonLd::canConvert<Reservation>(item)) {
        const auto res = JsonLd::convert<Reservation>(item);
        return !res.priceCurrency().isEmpty() ? res.priceCurrency() : PriceUtil::currency(res.reservedTicket());
    }

    if (JsonLd::isA<Ticket>(item)) {
        return item.value<Ticket>().priceCurrency();
    }

    return {};
}

void PriceUtil::setPrice(QVariant &item, double price, const QString &currency)
{
    // TODO check if there is a price on a nested ticket already, and set/replace that
    JsonLdDocument::writeProperty(item, "totalPrice", price);
    JsonLdDocument::writeProperty(item, "priceCurrency", currency);
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

int PriceUtil::decimalCount(QStringView currency)
{
    const auto it = std::lower_bound(std::begin(currency_decimals_map), std::end(currency_decimals_map), currency, [](const auto &m, QStringView isoCode) {
        return QLatin1String(m.isoCode, 3) < isoCode;
    });
    if (it != std::end(currency_decimals_map) && QLatin1String((*it).isoCode, 3) == currency) {
        return (*it).decimals;
    }
    return 2;
}

int PriceUtil::decimalCount(const QString &currency)
{
    return PriceUtil::decimalCount(QStringView(currency));
}

#include "moc_priceutil.cpp"
