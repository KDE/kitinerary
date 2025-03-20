/*
    SPDX-FileCopyrightText: 2025 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "uic9183flex.h"

#include <optional>

using namespace Qt::Literals;
using namespace KItinerary;

namespace KItinerary {
class Uic9183FlexPrivate : public QSharedData
{
public:
    std::optional<Fcb::UicRailTicketData> m_data;
    Uic9183Block m_block;
};
}

Uic9183Flex::Uic9183Flex() = default;
Uic9183Flex::Uic9183Flex(const Uic9183Block &block)
    : d(new Uic9183FlexPrivate)
{
    if (block.version() == 3) {
        if (auto fcb = Fcb::v3::UicRailTicketData(block); fcb.isValid()) {
            d->m_data = std::move(fcb);
            d->m_block = block;
            return;
        }
    }
    if (block.version() == 2) {
        if (auto fcb = Fcb::v2::UicRailTicketData(block); fcb.isValid()) {
            d->m_data = std::move(fcb);
            d->m_block = block;
            return;
        }
    }
    if (auto fcb = Fcb::v13::UicRailTicketData(block); fcb.isValid()) {
        d->m_data = std::move(fcb);
        d->m_block = block;
    }
}

Uic9183Flex::Uic9183Flex(const Uic9183Flex&) = default;
Uic9183Flex& Uic9183Flex::operator=(const Uic9183Flex&) = default;
Uic9183Flex::~Uic9183Flex() = default;

bool Uic9183Flex::isValid() const
{
    return d && !d->m_block.isNull() && d->m_data.has_value();
}

QDateTime Uic9183Flex::issuingDateTime() const
{
    return isValid() ? std::visit([](auto &&data) { return data.issuingDetail.issueingDateTime(); }, *d->m_data) : QDateTime();
}

bool Uic9183Flex::hasTransportDocument() const
{
    return isValid() && !std::visit([](auto &&data) { return data.transportDocument.empty(); }, *d->m_data);
}

QList<QVariant>Uic9183Flex::transportDocuments() const
{
    return isValid() ? std::visit([](auto &&data) {
        QList<QVariant> l;
        l.reserve(data.transportDocument.size());
        std::transform(data.transportDocument.begin(), data.transportDocument.end(), std::back_inserter(l), [](const auto &doc) { return doc.ticket; });
        return l;
    }, *d->m_data) : QList<QVariant>();
}

const Fcb::UicRailTicketData& Uic9183Flex::fcb() const
{
    return *d->m_data;
}

QVariant Uic9183Flex::fcbVariant() const
{
    return isValid() ? std::visit([](auto &&data) { return QVariant::fromValue(data); }, *d->m_data) : QVariant();
}

#include "moc_uic9183flex.cpp"
