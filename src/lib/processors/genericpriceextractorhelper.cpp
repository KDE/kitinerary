/*
   SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "genericpriceextractorhelper_p.h"

#include <text/pricefinder_p.h>

#include <KItinerary/ExtractorDocumentNode>
#include <KItinerary/ExtractorResult>
#include <KItinerary/JsonLdDocument>
#include <KItinerary/PriceUtil>
#include <KItinerary/Reservation>
#include <KItinerary/Ticket>

using namespace KItinerary;

static bool isApplicableForPrice(const QVariant &res)
{
    if (!PriceUtil::canHavePrice(res) || PriceUtil::hasPrice(res)) {
        return false;
    }

    // marker for extractor scripts to skip generic extraction
    const auto currency = JsonLdDocument::readProperty(res, "priceCurrency").toString();
    if (currency.isEmpty() && !currency.isNull()) {
        return false;
    }

    if (JsonLd::canConvert<Reservation>(res)) {
        const auto r = JsonLd::convert<Reservation>(res);
        if (r.reservationStatus() != Reservation::ReservationConfirmed) { // ignore cancellations
            return false;
        }
    }

    if (JsonLd::isA<FlightReservation>(res) && !res.value<FlightReservation>().reservedTicket().value<Ticket>().ticketToken().isEmpty()) { // ignore flight boarding passes
        return false;
    }

    if (JsonLd::isA<RentalCarReservation>(res)) { // car rental reservations are full of expensive insurance scam
        return false;
    }

    return true;
}

void GenericPriceExtractorHelper::postExtract(const QString &text, ExtractorDocumentNode &node)
{
    if (node.result().isEmpty()) {
        return;
    }

    PriceFinder priceFinder;
    std::vector<PriceFinder::Result> prices;
    priceFinder.findAll(text, prices);
    if (prices.empty() || !priceFinder.isSingleCurrency(prices)) {
        return;
    }

    auto results = node.result().result();
    if (std::none_of(results.begin(), results.end(), isApplicableForPrice)) {
        return;
    }

    // ambigious: could be one price for each reservation or the same total for all
    if (results.size() > 1 && prices.size() > 1) {
        return;
    }

    const auto price = priceFinder.highest(prices);
    if (!price.hasResult()) {
        return;
    }

    for (auto &r : results) {
        if (!isApplicableForPrice(r)) {
            continue;
        }
        PriceUtil::setPrice(r, price.value, price.currency);
    }
    node.setResult(std::move(results));
}
