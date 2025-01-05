/*
    SPDX-FileCopyrightText: 2025 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "uic9183flex.h"

using namespace KItinerary;

Uic9183Flex::Uic9183Flex() = default;

Uic9183Flex::Uic9183Flex(const Uic9183Block &block)
{
    auto fcb = Fcb::UicRailTicketData(block);
    if (fcb.isValid()) {
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
    return isValid() ? m_data.issuingDetail.issueingDateTime() : QDateTime();
}

bool Uic9183Flex::hasTransportDocument() const
{
    return isValid() && !m_data.transportDocument.empty();
}

QList<QVariant>Uic9183Flex::transportDocuments() const
{
    QList<QVariant> l;
    l.reserve(m_data.transportDocument.size());
    std::transform(m_data.transportDocument.begin(), m_data.transportDocument.end(), std::back_inserter(l), [](const auto &doc) { return doc.ticket; });
    return l;
}

Fcb::UicRailTicketData Uic9183Flex::fcb() const
{
    return m_data;
}

QVariant Uic9183Flex::fcbVariant() const
{
    return QVariant::fromValue(m_data);
}

#include "moc_uic9183flex.cpp"
