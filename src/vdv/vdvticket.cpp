/*
    Copyright (C) 2019 Volker Krause <vkrause@kde.org>

    This program is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This program is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "vdvticket.h"
#include "vdvdata_p.h"

#include <QDebug>

using namespace KItinerary;

namespace KItinerary {
class VdvTicketPrivate : public QSharedData
{
public:
    QByteArray m_data;
};
}


VdvTicket::VdvTicket()
    : d(new VdvTicketPrivate)
{
}

VdvTicket::VdvTicket(const QByteArray &data)
    : d(new VdvTicketPrivate)
{
    qDebug() << data.toHex() << data.size();
    if ((unsigned)data.size() < sizeof(VdvTicketHeader)) {
        qWarning() << "Ticket data too small";
        return;
    }

    d->m_data = data;
    const auto hdr = reinterpret_cast<const VdvTicketHeader*>(d->m_data.constData());
    qDebug() << qFromBigEndian(hdr->ticketId) << qFromBigEndian(hdr->kvpOrgId) << qFromBigEndian(hdr->productId) << qFromBigEndian(hdr->pvOrgId);
    qDebug() << "begin:" << beginDateTime();
    qDebug() << "end:" << endDateTime();

    // iterate over TLV blocks
    int offset = sizeof(VdvTicketHeader);
    while (offset < d->m_data.size() - 1) {
        qDebug() << "tag:" << (uint8_t)d->m_data[offset] << "size:" << (uint8_t)d->m_data[offset + 1] << "remaining:" << (d->m_data.size() - offset - (uint8_t)d->m_data[offset + 1]);
        offset += (uint8_t)d->m_data[offset + 1] + 2;
    }
}

VdvTicket::VdvTicket(const VdvTicket&) = default;
VdvTicket::~VdvTicket() = default;
VdvTicket& VdvTicket::operator=(const VdvTicket&) = default;

static QDateTime dtCompactToQdt(const VdvDateTimeCompact &dtc)
{
    return QDateTime({dtc.year(), dtc.month(), dtc.day()}, {dtc.hour(), dtc.minute(), dtc.second()});
}

QDateTime VdvTicket::beginDateTime() const
{
    if (d->m_data.isEmpty()) {
        return {};
    }

    const auto hdr = reinterpret_cast<const VdvTicketHeader*>(d->m_data.constData());
    return dtCompactToQdt(hdr->beginDt);
}

QDateTime KItinerary::VdvTicket::endDateTime() const
{
    if (d->m_data.isEmpty()) {
        return {};
    }

    const auto hdr = reinterpret_cast<const VdvTicketHeader*>(d->m_data.constData());
    return dtCompactToQdt(hdr->endDt);
}
