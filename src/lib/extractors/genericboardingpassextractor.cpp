/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "genericboardingpassextractor.h"
#include "flightutil_p.h"
#include "locationutil.h"
#include "logging.h"
#include "stringutil.h"

#include "knowledgedb/airportdb.h"
#include "knowledgedb/airportnametokenizer_p.h"
#include "pdf/pdfdocument.h"
#include "text/terminalfinder_p.h"
#include "text/timefinder_p.h"

#include <KItinerary/ExtractorDocumentNode>
#include <KItinerary/ExtractorResult>
#include <KItinerary/Flight>
#include <KItinerary/Reservation>

#include <QDebug>
#include <QTimeZone>

#include <chrono>
#include <unordered_map>

using namespace Qt::Literals;
using namespace KItinerary;

constexpr inline auto BOARDING_TO_DEPARTURE_MIN = std::chrono::minutes(20);
constexpr inline auto BOARDING_TO_DEPARTURE_MAX = std::chrono::minutes(75);
constexpr inline auto CHECKIN_TO_BOARDING_MIN = std::chrono::minutes(0);
constexpr inline auto CHECKIN_TO_BOARDING_MAX = std::chrono::minutes(35);
constexpr inline auto BOARDING_TO_GATE_CLOSE_MIN = std::chrono::minutes(15);
constexpr inline auto BOARDING_TO_GATE_CLOSE_MAX = std::chrono::minutes(30);
constexpr inline auto GATE_CLOSE_TO_DEPARTURE_MIN = std::chrono::minutes(10);
constexpr inline auto GATE_CLOSE_TO_DEPARTURE_MAX = std::chrono::minutes(15);
constexpr inline auto MINIMUM_FLIGHT_TIME = std::chrono::minutes(60);

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

[[nodiscard]] static int airportDistance(KnowledgeDb::IataCode from, KnowledgeDb::IataCode to)
{
    const auto fromCoord = KnowledgeDb::coordinateForAirport(from);
    const auto toCoord = KnowledgeDb::coordinateForAirport(to);
    if (!fromCoord.isValid() || !toCoord.isValid()) {
        return std::numeric_limits<int>::max();
    }
    return LocationUtil::distance({fromCoord.latitude, fromCoord.longitude}, {toCoord.latitude, toCoord.longitude});
}

[[nodiscard]] static bool isPlausibleBoardingTime(const QDateTime &boarding, const QDateTime &departure)
{
    const std::chrono::seconds boardingToDep(boarding.secsTo(departure));
    return boardingToDep >= BOARDING_TO_DEPARTURE_MIN && boardingToDep <= BOARDING_TO_DEPARTURE_MAX;
}

[[nodiscard]] static std::chrono::seconds flightDuration(const QDateTime &fromTime, const QDateTime &toTime, KnowledgeDb::IataCode from, KnowledgeDb::IataCode to)
{
    // times are local, so convert them to the right timezone first
    auto fromDt = fromTime;
    fromDt.setTimeZone(KnowledgeDb::timezoneForAirport(from));
    auto toDt = toTime;
    toDt.setTimeZone(KnowledgeDb::timezoneForAirport(to));
    return std::chrono::seconds(fromDt.secsTo(toDt));
}

[[nodiscard]] static bool isPlausibleFlightTime(const QDateTime &fromTime, const QDateTime &toTime, KnowledgeDb::IataCode from, KnowledgeDb::IataCode to)
{
    const auto duration = flightDuration(fromTime, toTime, from, to);
    if (duration < MINIMUM_FLIGHT_TIME) {
        return false;
    }

    const auto distance = airportDistance(from, to);
    return FlightUtil::isPlausibleDistanceForDuration(distance, duration);
}

[[nodiscard]] static bool isPlausibleCheckinClose(const QDateTime &checkinClose, const QDateTime &boarding)
{
    const std::chrono::seconds d(checkinClose.secsTo(boarding));
    return d >= CHECKIN_TO_BOARDING_MIN && d <= CHECKIN_TO_BOARDING_MAX;
}

[[nodiscard]] static bool isPlausibleGateClose(const QDateTime &boarding, const QDateTime &gateClose, const QDateTime &departure)
{
    const std::chrono::seconds gateOpen(boarding.secsTo(gateClose));
    const std::chrono::seconds gateCloseToDep(gateClose.secsTo(departure));

    return gateOpen >= BOARDING_TO_GATE_CLOSE_MIN && gateOpen <= BOARDING_TO_GATE_CLOSE_MAX
        && gateCloseToDep >= GATE_CLOSE_TO_DEPARTURE_MIN && gateCloseToDep <= GATE_CLOSE_TO_DEPARTURE_MAX;
}

[[nodiscard]] static bool conflictIfSet(const QDateTime &lhs, const QDateTime &rhs)
{
    return lhs.isValid() && rhs.isValid() && lhs != rhs;
}

static void applyFlightTimes(QList<QVariant> &result, const QDateTime &boarding, const QDateTime &dep, const QDateTime &arr)
{
    for (auto &res : result) {
        auto flightRes = res.value<FlightReservation>();
        auto flight = flightRes.reservationFor().value<Flight>();

        // check if already set times match, otherwise discard the entire set
        if (conflictIfSet(flight.boardingTime(), boarding) || conflictIfSet(flight.departureTime(), dep) || conflictIfSet(flight.arrivalTime(), arr)) {
            continue;
        }

        // apply not yet set times
        if (!flight.boardingTime().isValid() && boarding.isValid()) {
            flight.setBoardingTime(boarding);
        }
        if (!flight.departureTime().isValid() && dep.isValid()) {
            flight.setDepartureTime(dep);
        }
        if (!flight.arrivalTime().isValid() && arr.isValid()) {
            flight.setArrivalTime(arr);
        }
        flightRes.setReservationFor(flight);
        res = flightRes;
    }
}

ExtractorResult GenericBoardingPassExtractor::extract(const ExtractorDocumentNode &node, [[maybe_unused]] const ExtractorEngine *engine) const
{
    static TerminalFinder terminalFinder(u"^", u"(?=\\b|\\s|$)");

    QList<QVariant> fullResult;

    const auto pdf = node.content<PdfDocument*>();

    std::vector<ExtractorDocumentNode> bcbpNodes;
    m_filter.allMatches(node, bcbpNodes);
    bcbpNodes.erase(std::remove_if(bcbpNodes.begin(), bcbpNodes.end(), [](const auto &node) {
        return node.location().userType() != QMetaType::Int || node.result().isEmpty();
    }), bcbpNodes.end());
    std::sort(bcbpNodes.begin(), bcbpNodes.end(), [](const auto &lhs, const auto &rhs) { return lhs.location().toInt() < rhs.location().toInt(); });

    for (auto it = bcbpNodes.begin(); it != bcbpNodes.end(); ++it) {
        QDate departureDay;
        KnowledgeDb::IataCode from, to;
        QList<QVariant> result;

        // 1 determine which airports we need to look for on the same page
        const auto pageNum = (*it).location().toInt();
        std::unordered_map<KnowledgeDb::IataCode, QStringList> airportNames;
        std::unordered_map<KnowledgeDb::IataCode, QString> terminalNames;
        for (auto it2 = it; it2 != bcbpNodes.end() && (*it2).location().toInt() == pageNum; ++it2) {
            const auto flightReservations = (*it).result().result();
            for (const auto &flightRes : flightReservations) {
                const auto flight = flightRes.value<FlightReservation>().reservationFor().value<Flight>();
                if (!flight.departureAirport().iataCode().isEmpty()) {
                    from = KnowledgeDb::IataCode{flight.departureAirport().iataCode()};
                    airportNames[from] = QStringList();
                    terminalNames[from] = QString();
                }
                if (!flight.arrivalAirport().iataCode().isEmpty()) {
                    to = KnowledgeDb::IataCode{flight.arrivalAirport().iataCode()};
                    airportNames[to] = QStringList();
                    terminalNames[to] = QString();
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
            if (s.compare("international"_L1, Qt::CaseInsensitive) == 0 || s.compare("intl"_L1, Qt::CaseInsensitive) == 0) {
              qCDebug(Log) << "  ignoring" << s;
              continue;
            }

            // IATA code of one of the airports
            if (const auto code = KnowledgeDb::IataCode(s); !s.isNull() && airportNames.find(KnowledgeDb::IataCode{s}) != airportNames.end()) {
                // also look for terminal information after the IATA code itself
                const auto offset = s.size() + s.data() - pageText.data();
                const auto res = terminalFinder.find(QStringView(pageText).mid(offset));
                if (res.hasResult() && res.name != s.toString()) {
                    terminalNames[code] = res.name;
                }

                qCDebug(Log) << "  found own IATA code" << s;
                continue;
            }

            const auto iataCodes = KnowledgeDb::iataCodesFromName(s);
            for (const auto code : iataCodes) {
                auto it2 = airportNames.find(code);
                if (it2 != airportNames.end()) {
                    qCDebug(Log) << "  found candidate:" << s << iataCodes;
                    mergeOrAppend((*it2).second, s);

                    // look for a following terminal name at the position after s
                    const auto offset = s.size() + s.data() - pageText.data();
                    const auto res = terminalFinder.find(QStringView(pageText).mid(offset));
                    if (res.hasResult() && res.name != code.toString()) {
                        terminalNames[(*it2).first] = res.name;
                    }
                }
            }
        }

        // 3 augment the results with what we found
        const auto flightReservations = (*it).result().result();
        for (const auto &res : flightReservations) {
            auto flightRes = res.value<FlightReservation>();
            auto flight = flightRes.reservationFor().value<Flight>();
            auto airport = flight.departureAirport();
            if (airport.name().isEmpty()) {
                airport.setName(airportNames[KnowledgeDb::IataCode{airport.iataCode()}].join(QLatin1Char(' ')));
            }
            flight.setDepartureAirport(airport);
            if (flight.departureTerminal().isEmpty()) {
                flight.setDepartureTerminal(terminalNames[KnowledgeDb::IataCode{airport.iataCode()}]);
            }
            airport = flight.arrivalAirport();
            if (airport.name().isEmpty()) {
                airport.setName(airportNames[KnowledgeDb::IataCode{airport.iataCode()}].join(QLatin1Char(' ')));
            }
            flight.setArrivalAirport(airport);
            if (flight.arrivalTerminal().isEmpty()) {
                flight.setArrivalTerminal(terminalNames[KnowledgeDb::IataCode{airport.iataCode()}]);
            }
            flightRes.setReservationFor(flight);
            result.push_back(std::move(flightRes));
        }

        // 4 if there's only a single leg on this page, try to see if we can determine times
        if (airportNames.size() == 2) {
            TimeFinder timeFinder;
            timeFinder.find(pageText);
            std::vector<QDateTime> times;
            for (const auto &res : timeFinder.results()) {
                switch (res.dateTime.userType()) {
                    case QMetaType::QTime:
                        times.push_back(QDateTime(departureDay, res.dateTime.toTime()));
                        break;
                    case QMetaType::QDateTime:
                        if (res.dateTime.toDateTime().date() == departureDay) {
                            times.push_back(res.dateTime.toDateTime());
                        }
                        break;
                    case QMetaType::QDate:
                    default:
                        break;
                }
            }
            std::sort(times.begin(), times.end());
            times.erase(std::unique(times.begin(), times.end()), times.end());
            qCDebug(Log) << times;
            if (times.size() == 2) {
                // boarding/departure only, and on the same day
                if (isPlausibleBoardingTime(times[0], times[1]) && !isPlausibleFlightTime(times[0], times[1], from, to)) {
                    applyFlightTimes(result, times[0], times[1], {});
                }
            } else if (times.size() == 3) {
                // boarding/departure/arrival on the same day
                if (isPlausibleBoardingTime(times[0], times[1]) && isPlausibleFlightTime(times[1], times[2], from, to)) {
                    applyFlightTimes(result, times[0], times[1], times[2]);
                // boarding/departure on the same day, arrival on the next day
                } else if (isPlausibleBoardingTime(times[1], times[2]) && isPlausibleFlightTime(times[2], times[0].addDays(1), from, to)) {
                    applyFlightTimes(result, times[1], times[2], times[0].addDays(1));
                }
                // TODO handle boarding before midnight
                // departure/arrival/duration
                else if (isPlausibleFlightTime(times[1], times[2], from, to) && flightDuration(times[1], times[2], from, to) == std::chrono::minutes(times[0].time().hour() * 60 + times[0].time().minute())) {
                    applyFlightTimes(result, {}, times[1], times[2]);
                }
            } else if (times.size() == 4) {
                // baggage drop or checkin close/boarding/departure/arrival
                if (isPlausibleCheckinClose(times[0], times[1]) && isPlausibleBoardingTime(times[1], times[2]) && isPlausibleFlightTime(times[2], times[3], from, to)) {
                    applyFlightTimes(result, times[1], times[2], times[3]);
                    // boarding/gate close/departure/arrival
                } else if (isPlausibleBoardingTime(times[0], times[2]) && isPlausibleGateClose(times[0], times[1], times[2]) && isPlausibleFlightTime(times[2], times[3], from, to)) {
                    applyFlightTimes(result, times[0], times[2], times[3]);
                }
                // TODO across midnight variants
            }
        }

        fullResult += result;
    }

    return fullResult;
}
