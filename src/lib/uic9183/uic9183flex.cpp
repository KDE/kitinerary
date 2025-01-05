/*
    SPDX-FileCopyrightText: 2025 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "uic9183flex.h"

#include "era/fcbutil.h"

#include "variantvisitor_p.h"

#include <KItinerary/Place>

using namespace Qt::Literals;
using namespace KItinerary;

Uic9183Flex::Uic9183Flex() = default;

Uic9183Flex::Uic9183Flex(const Uic9183Block &block)
{
    if (block.version() == 3) {
        if (auto fcb = Fcb::v3::UicRailTicketData(block); fcb.isValid()) {
            m_data = std::move(fcb);
            m_block = block;
            return;
        }
    }
    if (auto fcb = Fcb::v13::UicRailTicketData(block); fcb.isValid()) {
        m_data = std::move(fcb);
        m_block = block;
    }
}

Uic9183Flex::Uic9183Flex(const Uic9183Flex&) = default;
Uic9183Flex& Uic9183Flex::operator=(const Uic9183Flex&) = default;
Uic9183Flex::~Uic9183Flex() = default;

bool Uic9183Flex::isValid() const
{
    return !m_block.isNull();
}

QDateTime Uic9183Flex::issuingDateTime() const
{
    return isValid() ? std::visit([](auto &&data) { return data.issuingDetail.issueingDateTime(); }, m_data) : QDateTime();
}

bool Uic9183Flex::hasTransportDocument() const
{
    return isValid() && !std::visit([](auto &&data) { return data.transportDocument.empty(); }, m_data);
}

QList<QVariant>Uic9183Flex::transportDocuments() const
{
    return std::visit([](auto &&data) {
        QList<QVariant> l;
        l.reserve(data.transportDocument.size());
        std::transform(data.transportDocument.begin(), data.transportDocument.end(), std::back_inserter(l), [](const auto &doc) { return doc.ticket; });
        return l;
    }, m_data);
}

const std::variant<Fcb::v13::UicRailTicketData, Fcb::v3::UicRailTicketData>& Uic9183Flex::fcb() const
{
    return m_data;
}

QVariant Uic9183Flex::fcbVariant() const
{
    return std::visit([](auto &&data) { return QVariant::fromValue(data); }, m_data);
}

void Uic9183Flex::readDepartureStation(const QVariant &doc, TrainStation &station)
{
    VariantVisitor([&station](auto &&data) {
        station.setName(data.fromStationNameUTF8);
        station.setIdentifier(FcbUtil::fromStationIdentifier(data));
    }).visit<Fcb::v13::ReservationData, Fcb::v13::OpenTicketData>(doc);
    fixStationCode(station);
}

void Uic9183Flex::readArrivalStation(const QVariant &doc, TrainStation &station)
{
    VariantVisitor([&station](auto &&data) {
        station.setName(data.toStationNameUTF8);
        station.setIdentifier(FcbUtil::toStationIdentifier(data));
    }).visit<Fcb::v13::ReservationData, Fcb::v13::OpenTicketData>(doc);
    fixStationCode(station);
}

void Uic9183Flex::fixStationCode(TrainStation &station)
{
    // UIC codes in Germany are wildly unreliable, there seem to be different
    // code tables in use by different operators, so we unfortunately have to ignore
    // those entirely
    if (station.identifier().startsWith("uic:80"_L1)) {
      PostalAddress addr;
      addr.setAddressCountry(u"DE"_s);
      station.setAddress(addr);
      station.setIdentifier(QString());
    }
}

#include "moc_uic9183flex.cpp"
