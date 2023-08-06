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
