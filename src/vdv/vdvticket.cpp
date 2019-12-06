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
#include "logging.h"

#include <QDebug>

using namespace KItinerary;

namespace KItinerary {
class VdvTicketPrivate : public QSharedData
{
public:
    QByteArray m_data;

    template <typename T> const T* productData() const;
};
}

template <typename T>
const T* VdvTicketPrivate::productData() const
{
    if (m_data.isEmpty()) {
        return nullptr;
    }

    const auto productBlock = reinterpret_cast<const VdvTicketProductData*>(m_data.constData() + sizeof(VdvTicketHeader));
    return productBlock->contentByTag<T>();
}

VdvTicket::VdvTicket()
    : d(new VdvTicketPrivate)
{
}

VdvTicket::VdvTicket(const QByteArray &data)
    : d(new VdvTicketPrivate)
{
    if (data.size() < MinimumTicketDataSize) {
        qCWarning(Log) << "Ticket data too small" << data.size();
        return;
    }

    static_assert(sizeof(VdvTicketHeader) < MinimumTicketDataSize, "");
    int offset = sizeof(VdvTicketHeader);

    const auto productBlock = reinterpret_cast<const VdvTicketProductData*>(data.constData() + offset);
    if (!productBlock->isValid() || productBlock->size() + offset > data.size()) {
        qCWarning(Log) << "Invalid product block" << productBlock->isValid() << productBlock->size() << offset << data.size();
        return;
    }
    offset += productBlock->size();

    const auto transactionBlock = reinterpret_cast<const VdvTicketTransactionData*>(data.constData() + offset);
    qDebug() << "transaction block:" << qFromBigEndian(transactionBlock->kvpOrgId);
    offset += sizeof(VdvTicketTransactionData);

    const auto prodTransactionBlock = reinterpret_cast<const VdvTicketProductTransactionData*>(data.constData() + offset);
    if (!prodTransactionBlock->isValid() || prodTransactionBlock->size() + offset > data.size()) {
        qCWarning(Log) << "Invalid product transaction block" << prodTransactionBlock->isValid() << prodTransactionBlock->size() << offset << data.size();
        return;
    }
    offset += prodTransactionBlock->size();

    const auto issueData = reinterpret_cast<const VdvTicketIssueData*>(data.constData() + offset);
    qDebug() << issueData->version << QByteArray((const char*)&issueData->samId, 3).toHex();
    offset += sizeof(VdvTicketIssueData);

    // 0 padding to reach at least 111 bytes
    offset += std::max(111 - offset - (int)sizeof(VdvTicketTrailer), 0);

    const auto trailer = reinterpret_cast<const VdvTicketTrailer*>(data.constData() + offset);
    if (memcmp(trailer->identifier, "VDV", 3) != 0) {
        qCWarning(Log) << "Invalid ticket trailer identifier:" << QByteArray(trailer->identifier, 3) << qFromBigEndian(trailer->version);
        return;
    }
    d->m_data = data;

#if 0
    const auto hdr = reinterpret_cast<const VdvTicketHeader*>(data.constData());
    qDebug() << qFromBigEndian(hdr->productId) << qFromBigEndian(hdr->pvOrgId);
    // iterate over TLV content
    auto tlv = productBlock->first();
    while (tlv) {
        qDebug() << "tag:" << tlv->tag << "size:" << tlv->contentSize() << "content:" << QByteArray((const char*)tlv->contentData(), tlv->contentSize()).toHex();
        tlv = productBlock->next(tlv);
    }
    const auto basicData = productBlock->contentByTag<VdvTicketBasicData>();
    if (basicData) {
        qDebug() << "traveler type:" << basicData->travelerType;
    }
#endif
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

int VdvTicket::issuerId() const
{
    if (d->m_data.isEmpty()) {
        return 0;
    }

    const auto hdr = reinterpret_cast<const VdvTicketHeader*>(d->m_data.constData());
    return qFromBigEndian(hdr->kvpOrgId);
}

VdvTicket::ServiceClass VdvTicket::serviceClass() const
{
    const auto tlv = d->productData<VdvTicketBasicData>();
    if (!tlv) {
        return UnknownClass;
    }
    switch (tlv->serviceClass) {
        case 0:
            return UnknownClass;
        case 1:
            return FirstClass;
        case 2:
            return SecondClass;
        case 3:
            return FirstClassUpgrade;
    }
    qCDebug(Log) << "Unknown service class:" << tlv->serviceClass;
    return UnknownClass;
}

Person VdvTicket::person() const
{
    const auto tlv = d->productData<VdvTicketTravelerData>();
    if (!tlv) {
        return {};
    }
    qDebug() << "traveler:" << tlv->gender << QDate(tlv->birthDate.year(), tlv->birthDate.month(), tlv->birthDate.day()) << QByteArray(tlv->name(), tlv->nameSize());

    const auto len = strnlen(tlv->name(), tlv->nameSize()); // name field can contain null bytes
    if (len == 0) {
        return {};
    }

    const auto name = QString::fromUtf8(tlv->name(), len);

    Person p;
    const auto idxHash = name.indexOf(QLatin1Char('#'));
    const auto idxAt = name.indexOf(QLatin1Char('@'));

    // encoding as first#last
    if (idxHash > 0) {
        p.setFamilyName(name.mid(idxHash + 1));
        p.setGivenName(name.left(idxHash));
    }

    // encoding as f1<len>fn@l1<len>ln
    else if (idxAt > 0) {
        p.setFamilyName(name.at(idxAt + 1));
        p.setGivenName(name.at(0));
    }

    // unknown encoding
    else {
        p.setName(name);
    }

    return p;
}

QString VdvTicket::ticketNumber() const
{
    if (d->m_data.isEmpty()) {
        return {};
    }

    const auto hdr = reinterpret_cast<const VdvTicketHeader*>(d->m_data.constData());
    return QString::number(qFromBigEndian(hdr->ticketId));
}
