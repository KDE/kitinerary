/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "locationutil.h"
#include "locationutil_p.h"
#include "stringutil.h"

#include <KItinerary/BoatTrip>
#include <KItinerary/BusTrip>
#include <KItinerary/Event>
#include <KItinerary/Flight>
#include <KItinerary/Place>
#include <KItinerary/Reservation>
#include <KItinerary/TrainTrip>
#include <KItinerary/Visit>

#include <KContacts/Address>

#include <QDebug>
#include <QRegularExpression>
#include <QUrl>
#include <QUrlQuery>

#include <cmath>

using namespace KItinerary;

KContacts::Address LocationUtil::toAddress(const PostalAddress &addr)
{
    KContacts::Address a;
    a.setStreet(addr.streetAddress());
    a.setPostalCode(addr.postalCode());
    a.setLocality(addr.addressLocality());
    a.setRegion(addr.addressRegion());
    a.setCountry(addr.addressCountry());
    return a;
}

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
    return JsonLd::isA<FlightReservation>(res) || JsonLd::isA<TrainReservation>(res) || JsonLd::isA<BusReservation>(res) || JsonLd::isA<TaxiReservation>(res) || JsonLd::isA<BoatReservation>(res);
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
    if (JsonLd::isA<BoatReservation>(res)) {
        return  res.value<BoatReservation>().reservationFor().value<BoatTrip>().arrivalBoatTerminal();
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
    if (JsonLd::isA<BoatReservation>(res)) {
        return res.value<BoatReservation>().reservationFor().value<BoatTrip>().departureBoatTerminal();
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

GeoCoordinates LocationUtil::geoFromUrl(const QUrl &url)
{
    if (url.host().contains(QLatin1StringView("google"))) {
        QRegularExpression regExp(
            QStringLiteral("[/=@](-?\\d+\\.\\d+),(-?\\d+\\.\\d+)"));
        auto match = regExp.match(url.path());
        if (!match.hasMatch()) {
            match = regExp.match(url.query());
        }

        if (match.hasMatch()) {
            const auto latitude = match.capturedView(1).toDouble();
            const auto longitude = match.capturedView(2).toDouble();

            return GeoCoordinates{latitude, longitude};
        }
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
int LocationUtil::distance(double lat1, double lon1, double lat2, double lon2)
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

template <typename It>
static void advanceToNextRelevantChar(It &it, const It &end)
{
    while (((*it).isSpace() || (*it).isPunct()) && it != end) {
        ++it;
    }
}

static bool compareSpaceCaseInsenstive(const QString &lhs, const QString &rhs)
{
    auto lit = lhs.begin();
    auto rit = rhs.begin();
    while (true) {
        advanceToNextRelevantChar(lit, lhs.end());
        advanceToNextRelevantChar(rit, rhs.end());
        if (lit == lhs.end() || rit == rhs.end()) {
            break;
        }
        if ((*lit).toCaseFolded() != (*rit).toCaseFolded()) {
            return false;
        }
        ++lit;
        ++rit;
    }

    return lit == lhs.end() && rit == rhs.end();
}

static bool startsWithSpaceCaseInsenstive(QStringView string, QStringView prefix)
{
    if (prefix.size() > string.size()) {
        return false;
    }

    auto sit = string.begin();
    auto pit = prefix.begin();
    while (true) {
        advanceToNextRelevantChar(sit, string.end());
        advanceToNextRelevantChar(pit, prefix.end());
        if (sit == string.end() || pit == prefix.end()) {
            break;
        }
        if ((*sit).toCaseFolded() != (*pit).toCaseFolded()) {
            return false;
        }
        ++sit;
        ++pit;
    }

    return pit == prefix.end();

}

static bool hasCommonPrefix(QStringView lhs, QStringView rhs)
{
    // check for a common prefix
    bool foundSeparator = false;
    for (auto i = 0; i < std::min(lhs.size(), rhs.size()); ++i) {
        if (lhs[i].toCaseFolded() != rhs[i].toCaseFolded()) {
            return foundSeparator;
        }
        foundSeparator |= !lhs[i].isLetter();
    }

    return lhs.startsWith(rhs, Qt::CaseInsensitive) || rhs.startsWith(lhs, Qt::CaseInsensitive);
}

[[nodiscard]] static bool isStrictLongPrefix(QStringView lhs, QStringView rhs)
{
    // 17 is the maximum field length in RCT2
    if (lhs.size() < 17 || rhs.size() < 17) {
        return false;
    }
    if (startsWithSpaceCaseInsenstive(lhs, rhs)) {
        return lhs.at(rhs.size()).isLetter();
    }
    if (startsWithSpaceCaseInsenstive(rhs, lhs)) {
        return rhs.at(lhs.size()).isLetter();
    }
    return false;
}

static bool isSameLocationName(const QString &lhs, const QString &rhs, LocationUtil::Accuracy accuracy)
{
    if (lhs.isEmpty() || rhs.isEmpty()) {
        return false;
    }

    // actually equal
    if (lhs.compare(rhs, Qt::CaseInsensitive) == 0) {
        return true;
    }

    // check if any of the Unicode normalization approaches helps
    const auto lhsNormalized = stripDiacritics(lhs);
    const auto rhsNormalized = stripDiacritics(rhs);
    const auto lhsTransliterated = StringUtil::transliterate(lhs);
    const auto rhsTransliterated = StringUtil::transliterate(rhs);
    if (compareSpaceCaseInsenstive(lhsNormalized, rhsNormalized) || compareSpaceCaseInsenstive(lhsNormalized, rhsTransliterated)
        || compareSpaceCaseInsenstive(lhsTransliterated, rhsNormalized) || compareSpaceCaseInsenstive(lhsTransliterated, rhsTransliterated)) {
        return true;
    }

    // sufficiently long prefix that we can assume RCT2 field overflow
    if (isStrictLongPrefix(lhs, rhs)) {
        return true;
    }

    if (accuracy == LocationUtil::CityLevel) {
        // check for a common prefix
        return hasCommonPrefix(lhsNormalized, rhsNormalized) || hasCommonPrefix(lhsTransliterated, rhsTransliterated);
    }

    return false;
}

bool LocationUtil::isSameLocation(const QVariant &lhs, const QVariant &rhs, LocationUtil::Accuracy accuracy)
{
    const auto lhsGeo = geo(lhs);
    const auto rhsGeo = geo(rhs);
    const auto lhsAddr = address(lhs);
    const auto rhsAddr = address(rhs);

    const auto lhsIsTransportStop = JsonLd::isA<Airport>(lhs) || JsonLd::isA<TrainStation>(lhs) || JsonLd::isA<BusStation>(lhs);
    const auto rhsIsTransportStop = JsonLd::isA<Airport>(rhs) || JsonLd::isA<TrainStation>(rhs) || JsonLd::isA<BusStation>(rhs);
    const auto isNameComparable = lhsIsTransportStop && rhsIsTransportStop;

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
                if (d >= 50000) {
                    return false;
                }
                if (d < 2000) {
                    return true;
                }
                if (d < 50000 && (lhsAddr.addressLocality().isEmpty() || rhsAddr.addressLocality().isEmpty()) && (!isNameComparable || name(lhs).isEmpty() || name(rhs).isEmpty())) {
                    return true;
                }
                break;
        }
    }

    switch (accuracy) {
        case Exact:
        case WalkingDistance:
            if (!lhsAddr.streetAddress().isEmpty() && !rhsAddr.streetAddress().isEmpty() && !rhsAddr.addressLocality().isEmpty()) {
                if (!lhsAddr.postalCode().isEmpty() && !rhsAddr.postalCode().isEmpty() && lhsAddr.postalCode() != rhsAddr.postalCode()) {
                    return false;
                }
                return  (lhsAddr.streetAddress().startsWith(rhsAddr.streetAddress(), Qt::CaseInsensitive) || rhsAddr.streetAddress().startsWith(lhsAddr.streetAddress(), Qt::CaseInsensitive)) && lhsAddr.addressLocality() == rhsAddr.addressLocality();
            }
            break;
        case CityLevel:
            if (!lhsAddr.addressLocality().isEmpty() && !rhsAddr.addressLocality().isEmpty()) {
                return isSameLocationName(lhsAddr.addressLocality(), rhsAddr.addressLocality(), LocationUtil::Exact);
            }
            break;
    }

    return isSameLocationName(name(lhs), name(rhs), accuracy);
}

QUrl LocationUtil::geoUri(const QVariant &location)
{
    QUrl url;
    url.setScheme(QStringLiteral("geo"));

    const auto geo = LocationUtil::geo(location);
    if (geo.isValid()) {
        url.setPath(QString::number(geo.latitude()) + QLatin1Char(',') + QString::number(geo.longitude()));
        return url;
    }

    const auto addr = LocationUtil::address(location);
    if (!addr.isEmpty()) {
        url.setPath(QStringLiteral("0,0"));
        QUrlQuery query;
        query.addQueryItem(QStringLiteral("q"), toAddress(addr).formatted(KContacts::AddressFormatStyle::GeoUriQuery));
        url.setQuery(query);
        return url;
    }

    return {};
}
