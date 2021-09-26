/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "genericboardingpassextractor.h"
#include "flightutil_p.h"
#include "locationutil.h"
#include "logging.h"
#include "stringutil.h"

#include <knowledgedb/airportdb.h>
#include <knowledgedb/airportnametokenizer_p.h>
#include <pdf/pdfdocument.h>
#include <text/timefinder_p.h>

#include <KItinerary/ExtractorDocumentNode>
#include <KItinerary/ExtractorResult>
#include <KItinerary/Flight>
#include <KItinerary/Reservation>

#include <QDebug>
#include <QTimeZone>

#include <unordered_map>

using namespace KItinerary;

GenericBoardingPassExtractor::GenericBoardingPassExtractor()
{
    m_filter.setMimeType(QStringLiteral("internal/iata-bcbp"));
    m_filter.setScope(ExtractorFilter::Descendants);
}

GenericBoardingPassExtractor::~GenericBoardingPassExtractor() = default;

QString GenericBoardingPassExtractor::name() const
{
    return QStringLiteral("<Generic PDF Boarding Pass>");
}

bool GenericBoardingPassExtractor::canHandle(const ExtractorDocumentNode &node) const
{
    return node.content<PdfDocument*>() && m_filter.matches(node);
}

static void mergeOrAppend(QStringList &l, QStringView s)
{
    for (auto &n : l) {
        if (n.compare(s, Qt::CaseInsensitive)  == 0) {
            n = StringUtil::betterString(n, s).toString();
            return;
        }
    }
    l.push_back(s.toString());
}

static int airportDistance(KnowledgeDb::IataCode from, KnowledgeDb::IataCode to)
{
    const auto fromCoord = KnowledgeDb::coordinateForAirport(from);
    const auto toCoord = KnowledgeDb::coordinateForAirport(to);
    if (!fromCoord.isValid() || !toCoord.isValid()) {
        return std::numeric_limits<int>::max();
    }
    return LocationUtil::distance({fromCoord.latitude, fromCoord.longitude}, {toCoord.latitude, toCoord.longitude});
}

static bool isPlausibleBoardingTime(const QDateTime &boarding, const QDateTime &departure)
{
    return boarding < departure && boarding.secsTo(departure) <= 3600;
}

static bool isPlausibleFlightTimes(const std::vector<QDateTime> &times, KnowledgeDb::IataCode from, KnowledgeDb::IataCode to)
{
    if (!isPlausibleBoardingTime(times[0], times[1])) {
        return false;
    }
    const auto distance = airportDistance(from, to);

    // times are local, so convert them to the right timezone first
    auto fromDt = times[1];
    fromDt.setTimeZone(KnowledgeDb::timezoneForAirport(from));
    auto toDt = times[2];
    toDt.setTimeZone(KnowledgeDb::timezoneForAirport(to));

    const auto flightDuration = fromDt.secsTo(toDt);
    if (flightDuration < 3600) {
        return false;
    }
    return fromDt < toDt && FlightUtil::isPlausibleDistanceForDuration(distance, flightDuration);
}

ExtractorResult GenericBoardingPassExtractor::extract(const ExtractorDocumentNode &node, [[maybe_unused]] const ExtractorEngine *engine) const
{
    QVector<QVariant> fullResult;

    const auto pdf = node.content<PdfDocument*>();

    std::vector<ExtractorDocumentNode> bcbpNodes;
    m_filter.allMatches(node, bcbpNodes);
    std::remove_if(bcbpNodes.begin(), bcbpNodes.end(), [](const auto &node) {
        return node.location().type() != QVariant::Int || node.result().isEmpty();
    });
    std::sort(bcbpNodes.begin(), bcbpNodes.end(), [](const auto &lhs, const auto &rhs) { return lhs.location().toInt() < rhs.location().toInt(); });

    for (auto it = bcbpNodes.begin(); it != bcbpNodes.end(); ++it) {
        QDate departureDay;
        KnowledgeDb::IataCode from, to;
        QVector<QVariant> result;

        // 1 determine which airports we need to look for on the same page
        const auto pageNum = (*it).location().toInt();
        std::unordered_map<KnowledgeDb::IataCode, QStringList> airportNames;
        for (auto it2 = it; it2 != bcbpNodes.end() && (*it2).location().toInt() == pageNum; ++it2) {
            const auto flightReservations = (*it).result().result();
            for (const auto &flightRes : flightReservations) {
                const auto flight = flightRes.value<FlightReservation>().reservationFor().value<Flight>();
                if (!flight.departureAirport().iataCode().isEmpty()) {
                    from = KnowledgeDb::IataCode{flight.departureAirport().iataCode()};
                    airportNames[from] = QStringList();
                }
                if (!flight.arrivalAirport().iataCode().isEmpty()) {
                    to = KnowledgeDb::IataCode{flight.arrivalAirport().iataCode()};
                    airportNames[to] = QStringList();
                }
                departureDay = flight.departureDay();
            }
        }

        // 2 tokenize the page and scan for airport names
        const auto page = pdf->page(pageNum);
        qCDebug(Log) << "scanning page" << pageNum << "for airport names";
        const auto pageText = page.text();
        AirportNameTokenizer tokenizer(pageText);
        while (tokenizer.hasNext()) {
            const auto s = tokenizer.next();
            if (s.compare(QLatin1String("international"), Qt::CaseInsensitive) == 0 ||
               (s.size() == 3 && airportNames.find(KnowledgeDb::IataCode{s}) != airportNames.end()))
            {
                qCDebug(Log) << "  ignoring" << s;
                continue;
            }
            const auto iataCodes = KnowledgeDb::iataCodesFromName(s);
            for (const auto code : iataCodes) {
                auto it2 = airportNames.find(code);
                if (it2 != airportNames.end()) {
                    qCDebug(Log) << "  found candidate:" << s << iataCodes;
                    mergeOrAppend((*it2).second, s);
                }
            }
        }

        // 3 augment the results with what we found
        const auto flightReservations = (*it).result().result();
        for (const auto &res : flightReservations) {
            auto flightRes = res.value<FlightReservation>();
            auto flight = flightRes.reservationFor().value<Flight>();
            auto airport = flight.departureAirport();
            airport.setName(airportNames[KnowledgeDb::IataCode{airport.iataCode()}].join(QLatin1Char(' ')));
            flight.setDepartureAirport(airport);
            airport = flight.arrivalAirport();
            airport.setName(airportNames[KnowledgeDb::IataCode{airport.iataCode()}].join(QLatin1Char(' ')));
            flight.setArrivalAirport(airport);
            flightRes.setReservationFor(flight);
            result.push_back(std::move(flightRes));
        }

        // 4 if there's only a single leg on this page, try to see if we can determine times
        if (airportNames.size() == 2) {
            TimeFinder timeFinder;
            timeFinder.find(pageText);
            std::vector<QDateTime> times;
            for (const auto &res : timeFinder.results()) {
                switch (res.dateTime.type()) {
                    case QVariant::Time:
                        times.push_back(QDateTime(departureDay, res.dateTime.toTime()));
                        break;
                    case QVariant::DateTime:
                        if (res.dateTime.toDateTime().date() == departureDay) {
                            times.push_back(res.dateTime.toDateTime());
                        }
                        break;
                    case QVariant::Date:
                    default:
                        break;
                }
            }
            std::sort(times.begin(), times.end());
            times.erase(std::unique(times.begin(), times.end()), times.end());
            if (times.size() == 3) {
                // apply what we found
                if (isPlausibleFlightTimes(times, from, to)) {
                    for (auto &res : result) {
                        auto flightRes = res.value<FlightReservation>();
                        auto flight = flightRes.reservationFor().value<Flight>();
                        if (!flight.boardingTime().isValid()) {
                            flight.setBoardingTime(times[0]);
                        }
                        if (!flight.departureTime().isValid()) {
                            flight.setDepartureTime(times[1]);
                        }
                        if (!flight.arrivalTime().isValid()) {
                            flight.setArrivalTime(times[2]);
                        }
                        flightRes.setReservationFor(flight);
                        res = flightRes;
                    }
                }

                // TODO handle arrival past midnight
                // TODO handle boarding before midnight
            }
        }

        fullResult += result;
    }

    return fullResult;
}
