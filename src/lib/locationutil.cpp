/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "locationutil.h"

#include <KItinerary/BusTrip>
#include <KItinerary/Event>
#include <KItinerary/Flight>
#include <KItinerary/Place>
#include <KItinerary/Reservation>
#include <KItinerary/TrainTrip>
#include <KItinerary/Visit>

#include <QDebug>

#include <cmath>

using namespace KItinerary;

bool LocationUtil::isLocationChange(const QVariant &res)
{
    if (JsonLd::isA<RentalCarReservation>(res)) {
        const auto pickup = departureLocation(res);
        const auto dropoff = arrivalLocation(res);
        if (dropoff.value<Place>().name().isEmpty()) {
            return false;
        }
        return !isSameLocation(pickup, dropoff);
    }
    return JsonLd::isA<FlightReservation>(res) || JsonLd::isA<TrainReservation>(res) || JsonLd::isA<BusReservation>(res) || JsonLd::isA<TaxiReservation>(res);
}

QVariant LocationUtil::arrivalLocation(const QVariant &res)
{
    if (JsonLd::isA<FlightReservation>(res)) {
        return res.value<FlightReservation>().reservationFor().value<Flight>().arrivalAirport();
    }
    if (JsonLd::isA<TrainReservation>(res)) {
        return res.value<TrainReservation>().reservationFor().value<TrainTrip>().arrivalStation();
    }
    if (JsonLd::isA<BusReservation>(res)) {
        return res.value<BusReservation>().reservationFor().value<BusTrip>().arrivalBusStop();
    }
    if (JsonLd::isA<RentalCarReservation>(res)) {
        return res.value<RentalCarReservation>().dropoffLocation();
    }
    return {};
}

QVariant LocationUtil::departureLocation(const QVariant &res)
{
    if (JsonLd::isA<FlightReservation>(res)) {
        return res.value<FlightReservation>().reservationFor().value<Flight>().departureAirport();
    }
    if (JsonLd::isA<TrainReservation>(res)) {
        return res.value<TrainReservation>().reservationFor().value<TrainTrip>().departureStation();
    }
    if (JsonLd::isA<BusReservation>(res)) {
        return res.value<BusReservation>().reservationFor().value<BusTrip>().departureBusStop();
    }
    if (JsonLd::isA<RentalCarReservation>(res)) {
        return res.value<RentalCarReservation>().pickupLocation();
    }
    if (JsonLd::isA<TaxiReservation>(res)) {
        return res.value<TaxiReservation>().pickupLocation();
    }
    return {};
}

QVariant LocationUtil::location(const QVariant &res)
{
    if (JsonLd::isA<LodgingReservation>(res)) {
        return res.value<LodgingReservation>().reservationFor();
    }
    if (JsonLd::isA<FoodEstablishmentReservation>(res)) {
        return res.value<FoodEstablishmentReservation>().reservationFor();
    }
    if (JsonLd::isA<TouristAttractionVisit>(res)) {
        return res.value<TouristAttractionVisit>().touristAttraction();
    }
    if (JsonLd::isA<EventReservation>(res)) {
        return res.value<EventReservation>().reservationFor().value<Event>().location();
    }
    if (JsonLd::isA<RentalCarReservation>(res)) {
        return res.value<RentalCarReservation>().pickupLocation();
    }

    return {};
}

GeoCoordinates LocationUtil::geo(const QVariant &location)
{
    if (JsonLd::canConvert<Place>(location)) {
        return JsonLd::convert<Place>(location).geo();
    }
    if (JsonLd::canConvert<Organization>(location)) {
        return JsonLd::convert<Organization>(location).geo();
    }

    return {};
}

PostalAddress LocationUtil::address(const QVariant &location)
{
    if (JsonLd::canConvert<Place>(location)) {
        return JsonLd::convert<Place>(location).address();
    }
    if (JsonLd::canConvert<Organization>(location)) {
        return JsonLd::convert<Organization>(location).address();
    }

    return {};
}

QString LocationUtil::name(const QVariant &location)
{
    if (JsonLd::isA<Airport>(location)) {
        const auto airport = location.value<Airport>();
        return airport.name().isEmpty() ? airport.iataCode() : airport.name();
    }
    if (JsonLd::canConvert<Place>(location)) {
        return JsonLd::convert<Place>(location).name();
    }
    if (JsonLd::canConvert<Organization>(location)) {
        return JsonLd::convert<Organization>(location).name();
    }

    return {};
}

int LocationUtil::distance(const GeoCoordinates &coord1, const GeoCoordinates &coord2)
{
    return distance(coord1.latitude(), coord1.longitude(), coord2.latitude(), coord2.longitude());
}

// see https://en.wikipedia.org/wiki/Haversine_formula
int LocationUtil::distance(float lat1, float lon1, float lat2, float lon2)
{
    const auto degToRad = M_PI / 180.0;
    const auto earthRadius = 6371000.0; // in meters

    const auto d_lat = (lat1 - lat2) * degToRad;
    const auto d_lon = (lon1 - lon2) * degToRad;

    const auto a = pow(sin(d_lat / 2.0), 2) + cos(lat1 * degToRad) * cos(lat2 * degToRad) * pow(sin(d_lon / 2.0), 2);
    return 2.0 * earthRadius * atan2(sqrt(a), sqrt(1.0 - a));
}

// if the character has a canonical decomposition use that and skip the combining diacritic markers following it
// see https://en.wikipedia.org/wiki/Unicode_equivalence
// see https://en.wikipedia.org/wiki/Combining_character
static QString stripDiacritics(const QString &s)
{
    QString res;
    res.reserve(s.size());
    for (const auto &c : s) {
        if (c.decompositionTag() == QChar::Canonical) {
            res.push_back(c.decomposition().at(0));
        } else {
            res.push_back(c);
        }
    }
    return res;
}

// keep this ordered (see https://en.wikipedia.org/wiki/List_of_Unicode_characters)
struct {
    ushort key;
    const char* replacement;
} static const transliteration_map[] = {
    { u'ä', "ae" },
    { u'ö', "oe" },
    { u'ø', "oe" },
    { u'ü', "ue" }
};

static QString applyTransliterations(const QString &s)
{
    QString res;
    res.reserve(s.size());

    for (const auto c : s) {
        const auto it = std::lower_bound(std::begin(transliteration_map), std::end(transliteration_map), c, [](const auto &lhs, const auto rhs) {
            return QChar(lhs.key) < rhs;
        });
        if (it != std::end(transliteration_map) && QChar((*it).key) == c) {
            res += QString::fromUtf8((*it).replacement);
            continue;
        }

        if (c.decompositionTag() == QChar::Canonical) { // see above
            res += c.decomposition().at(0);
        } else {
            res += c;
        }
    }

    return res;
}

static bool isSameLocationName(const QString &lhs, const QString &rhs, LocationUtil::Accuracy accuracy)
{
    Q_UNUSED(accuracy) // TODO for city level we can strip station or airport suffixes for example

    if (lhs.isEmpty() || rhs.isEmpty()) {
        return false;
    }

    // actually equal
    if (lhs.compare(rhs, Qt::CaseInsensitive) == 0) {
        return true;
    }

    // check if any of the unicode normalization approaches helps
    const auto lhsNormalized = stripDiacritics(lhs);
    const auto rhsNormalized = stripDiacritics(rhs);
    const auto lhsTransliterated = applyTransliterations(lhs);
    const auto rhsTransliterated = applyTransliterations(rhs);
    return lhsNormalized.compare(rhsNormalized, Qt::CaseInsensitive) == 0 || lhsNormalized.compare(rhsTransliterated, Qt::CaseInsensitive) == 0
        || lhsTransliterated.compare(rhsNormalized, Qt::CaseInsensitive) == 0 || lhsTransliterated.compare(rhsTransliterated, Qt::CaseInsensitive) == 0;
}

bool LocationUtil::isSameLocation(const QVariant &lhs, const QVariant &rhs, LocationUtil::Accuracy accuracy)
{
    const auto lhsGeo = geo(lhs);
    const auto rhsGeo = geo(rhs);
    if (lhsGeo.isValid() && rhsGeo.isValid()) {
        const auto d = distance(lhsGeo, rhsGeo);
        switch (accuracy) {
            case Exact:
                return d < 100;
            case WalkingDistance:
            {
                // airports are large but we have no local transport there, so the distance threshold needs to be higher there
                const auto isAirport = JsonLd::isA<Airport>(lhs) || JsonLd::isA<Airport>(rhs);
                return d < (isAirport ? 2000 : 1000);
            }
            case CityLevel:
                return d < 50000;
                break;
        }
        return false;
    }

    const auto lhsAddr = address(lhs);
    const auto rhsAddr = address(rhs);
    switch (accuracy) {
        case Exact:
        case WalkingDistance:
            if (!lhsAddr.streetAddress().isEmpty() && !lhsAddr.addressLocality().isEmpty()) {
                return  lhsAddr.streetAddress() == rhsAddr.streetAddress() && lhsAddr.addressLocality() == rhsAddr.addressLocality();
            }
            break;
        case CityLevel:
            if (!lhsAddr.addressLocality().isEmpty()) {
                return lhsAddr.addressLocality() == rhsAddr.addressLocality();
            }
            break;
    }

    return isSameLocationName(name(lhs), name(rhs), accuracy);
}