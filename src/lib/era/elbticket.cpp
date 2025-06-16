/*
    SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "elbticket.h"

#include <QDebug>

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
    return segment.isValid() ? segment : ELBTicketSegment{};
}

QDate ELBTicket::emissionDate(const QDateTime &contextDate) const
{
    const auto y = contextDate.date().year() - contextDate.date().year() % 10 + year();
    const auto d = QDate(y, 1, 1).addDays(emissionDay() - 1);
    if (y > contextDate.date().year()) {
        return QDate(y - 10, 1, 1).addDays(emissionDay() - 1);
    }
    return d;
}

QDateTime ELBTicket::validFromDate(const QDateTime &contextDate) const
{
    QDate d(emissionDate(contextDate).year(), 1, 1);
    if (beginValidityDay() < emissionDay()) {
        d = d.addYears(1);
    }
    return d.addDays(beginValidityDay() - 1).startOfDay();
}

QDateTime ELBTicket::validUntilDate(const QDateTime &contextDate) const
{
    QDate d(emissionDate(contextDate).year(), 1, 1);
    if (beginValidityDay() < emissionDay()) {
        d = d.addYears(1);
    }
    if (endValidityDay() < beginValidityDay()) {
        d = d.addYears(1);
    }
    return d.addDays(endValidityDay() -1).startOfDay();
}

QString ELBTicket::rawData() const
{
    return QString::fromUtf8(m_data);
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

    return QByteArrayView(m_data.constData() + start, len).toInt();
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

    if (!ticket.segment1().isValid()) {
        return {};
    }

    return ticket;
}


ELBTicketSegment::~ELBTicketSegment() = default;

bool ELBTicketSegment::isValid() const
{
    if (m_ticket.m_data.size() < m_offset + 36) {
        return false;
    }

    return departureDay() > 0 && (classOfTransport() == QLatin1Char('1') || classOfTransport() == QLatin1Char('2'));
}

QDate ELBTicketSegment::departureDate(const QDateTime &contextDate) const
{
    QDate d(m_ticket.emissionDate(contextDate).year(), 1, 1);
    if (departureDay() < m_ticket.emissionDay()) {
        d = d.addYears(1);
    }
    return d.addDays(departureDay() - 1);
}

QString ELBTicketSegment::readString(int start, int len) const
{
    return m_ticket.readString(m_offset + start, len);
}

int ELBTicketSegment::readNumber(int start, int len) const
{
    return m_ticket.readNumber(m_offset + start, len);
}

#include "moc_elbticket.cpp"
