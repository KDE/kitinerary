/*
    SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "elbticket.h"

using namespace KItinerary;

ELBTicket::~ELBTicket() = default;

ELBTicketSegment ELBTicket::segment1() const
{
    ELBTicketSegment segment;
    segment.m_ticket = *this;
    segment.m_offset = 49;
    return segment;
}

ELBTicketSegment ELBTicket::segment2() const
{
    ELBTicketSegment segment;
    segment.m_ticket = *this;
    segment.m_offset = 85;
    return segment;
}

QString ELBTicket::readString(int start, int len) const
{
    if (m_data.size() <= start + len) {
        return {};
    }

    return QString::fromUtf8(m_data.constData() + start, len);
}

int ELBTicket::readNumber(int start, int len) const
{
    if (m_data.size() <= start + len) {
        return -1;
    }

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    return QByteArrayView(m_data.constData() + start, len).toInt();
#else
    return QByteArray(m_data.constData() + start, len).toInt();
#endif
}


bool ELBTicket::maybeELBTicket(const QByteArray &data)
{
    return data.size() >= 85 && data[0] == 'e';
}

std::optional<ELBTicket> ELBTicket::parse(const QByteArray &data)
{
    if (!maybeELBTicket(data)) {
        return {};
    }

    // check validity as strictly as possible, as ELB has no sufficiently unique marker to reliably detect it
    // and we need to be tolerant to trailing content as there are signed variants of this in use

    if (std::any_of(data.begin(), data.end(), [](char c) { return c < ' ' || c > '~'; }) || (data[19] != '0' && data[19] != '1')) {
        return {};
    }

    ELBTicket ticket;
    ticket.m_data = data;
    if (ticket.barcodeVersion() != 2 || ticket.sequenceNumberCurrent() < 1 || ticket.sequenceNumberCurrent() < ticket.sequenceNumberTotal()) {
        return {};
    }
    if (std::any_of(data.constData() + 35, data.constData() + 49, [](char c) { return c < '0' || c > '9'; })) {
        return {};
    }

    return ticket;
}


ELBTicketSegment::~ELBTicketSegment() = default;

QString ELBTicketSegment::readString(int start, int len) const
{
    return m_ticket.readString(m_offset + start, len);
}
