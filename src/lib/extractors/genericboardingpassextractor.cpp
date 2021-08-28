/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "genericboardingpassextractor.h"
#include "logging.h"
#include "stringutil.h"

#include <knowledgedb/airportdb.h>
#include <knowledgedb/airportnametokenizer_p.h>
#include <pdf/pdfdocument.h>

#include <KItinerary/ExtractorDocumentNode>
#include <KItinerary/ExtractorResult>
#include <KItinerary/Flight>
#include <KItinerary/Reservation>

#include <QDebug>

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

ExtractorResult GenericBoardingPassExtractor::extract(const ExtractorDocumentNode &node, [[maybe_unused]] const ExtractorEngine *engine) const
{
    QVector<QVariant> result;

    const auto pdf = node.content<PdfDocument*>();

    std::vector<ExtractorDocumentNode> bcbpNodes;
    m_filter.allMatches(node, bcbpNodes);
    std::remove_if(bcbpNodes.begin(), bcbpNodes.end(), [](const auto &node) {
        return node.location().type() != QVariant::Int || node.result().isEmpty();
    });
    std::sort(bcbpNodes.begin(), bcbpNodes.end(), [](const auto &lhs, const auto &rhs) { return lhs.location().toInt() < rhs.location().toInt(); });

    for (auto it = bcbpNodes.begin(); it != bcbpNodes.end(); ++it) {
        // 1 determine which airports we need to look for on the same page
        const auto pageNum = (*it).location().toInt();
        std::unordered_map<KnowledgeDb::IataCode, QStringList> airportNames;
        for (auto it2 = it; it2 != bcbpNodes.end() && (*it2).location().toInt() == pageNum; ++it2) {
            const auto flightReservations = (*it).result().result();
            for (const auto &flightRes : flightReservations) {
                const auto flight = flightRes.value<FlightReservation>().reservationFor().value<Flight>();
                if (!flight.departureAirport().iataCode().isEmpty()) {
                    airportNames[KnowledgeDb::IataCode{flight.departureAirport().iataCode()}] = QStringList();
                }
                if (!flight.arrivalAirport().iataCode().isEmpty()) {
                    airportNames[KnowledgeDb::IataCode{flight.arrivalAirport().iataCode()}] = QStringList();
                }
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
    }

    return result;
}
