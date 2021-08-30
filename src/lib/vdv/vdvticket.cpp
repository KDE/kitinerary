/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "vdvticket.h"
#include "vdvticketcontent.h"
#include "vdvdata_p.h"
#include "logging.h"

#include "../tlv/berelement.h"

#include <QDebug>

using namespace KItinerary;

namespace KItinerary {
class VdvTicketPrivate : public QSharedData
{
public:
    QByteArray m_data;
    QByteArray m_rawData;

    BER::Element productElement(uint32_t type) const;
    template <typename T> const T* productData(uint32_t type = T::Tag) const;
};
}

BER::Element VdvTicketPrivate::productElement(uint32_t type) const
{
    const auto productElement = BER::TypedElement<TagTicketProductData>(m_data, sizeof(VdvTicketHeader));
    if (!productElement.isValid()) {
        return {};
    }
    return productElement.find(type);
}

template <typename T>
const T* VdvTicketPrivate::productData(uint32_t type) const
{
    const auto elem = productElement(type);
    if (elem.isValid()) {
        return elem.template contentAt<T>();
    }
    return nullptr;
}

VdvTicket::VdvTicket()
    : d(new VdvTicketPrivate)
{
}

VdvTicket::VdvTicket(const QByteArray &data, const QByteArray &rawData)
    : d(new VdvTicketPrivate)
{
    if (data.size() < MinimumTicketDataSize) {
        qCWarning(Log) << "Ticket data too small" << data.size();
        return;
    }

    static_assert(sizeof(VdvTicketHeader) < MinimumTicketDataSize, "");
    int offset = sizeof(VdvTicketHeader);

    const auto productBlock = BER::TypedElement<TagTicketProductData>(data, offset);
    if (!productBlock.isValid() || productBlock.size() + offset > data.size()) {
        qCWarning(Log) << "Invalid product block" << productBlock.isValid() << productBlock.size() << offset << data.size();
        return;
    }
    offset += productBlock.size();
    offset += sizeof(VdvTicketCommonTransactionData);

    const auto prodTransactionBlock = BER::TypedElement<TagTicketProductTransactionData>(data, offset);
    if (!prodTransactionBlock.isValid()) {
        qCWarning(Log) << "Invalid product transaction block" << prodTransactionBlock.isValid() << offset << data.size();
        return;
    }
    offset += prodTransactionBlock.size();
    offset += sizeof(VdvTicketIssueData);

    // 0 padding to reach at least 111 bytes
    offset += std::max(111 - offset - (int)sizeof(VdvTicketTrailer), 0);

    const auto trailer = reinterpret_cast<const VdvTicketTrailer*>(data.constData() + offset);
    if (memcmp(trailer->identifier, "VDV", 3) != 0) {
        qCWarning(Log) << "Invalid ticket trailer identifier:" << QByteArray(trailer->identifier, 3) << trailer->version;
        return;
    }
    d->m_data = data;
    d->m_rawData = rawData;
}

VdvTicket::VdvTicket(const VdvTicket&) = default;
VdvTicket::~VdvTicket() = default;
VdvTicket& VdvTicket::operator=(const VdvTicket&) = default;

QDateTime VdvTicket::beginDateTime() const
{
    const auto hdr = header();
    return hdr ? hdr->validityBegin : QDateTime();
}

QDateTime KItinerary::VdvTicket::endDateTime() const
{
    const auto hdr = header();
    return hdr ? hdr->validityEnd : QDateTime();
}

int VdvTicket::issuerId() const
{
    const auto hdr = header();
    return hdr ? hdr->kvpOrgId : 0;
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
    const auto elem = d->productElement(VdvTicketTravelerData::Tag);
    const auto tlv = elem.isValid() ? elem.contentAt<VdvTicketTravelerData>() : nullptr;
    if (!tlv) {
        return {};
    }

    const auto len = strnlen(tlv->name(), tlv->nameSize(elem.contentSize())); // name field can contain null bytes
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
    const auto hdr = header();
    return hdr ? QString::number(hdr->ticketId) : QString();
}

const VdvTicketHeader* VdvTicket::header() const
{
    return d->m_data.isEmpty() ? nullptr : reinterpret_cast<const VdvTicketHeader*>(d->m_data.constData());
}

BER::Element VdvTicket::productData() const
{
    const auto productElement = BER::Element(d->m_data, sizeof(VdvTicketHeader));
    return (productElement.isValid() && productElement.type() == TagTicketProductData) ? productElement : BER::Element();
}

const VdvTicketCommonTransactionData* VdvTicket::commonTransactionData() const
{
    return d->m_data.isEmpty() ? nullptr :
        reinterpret_cast<const VdvTicketCommonTransactionData*>(d->m_data.constData() + sizeof(VdvTicketHeader) + productData().size());
}

BER::Element VdvTicket::productSpecificTransactionData() const
{
    const auto offset = sizeof(VdvTicketHeader) + productData().size() + sizeof(VdvTicketCommonTransactionData);
    const auto elem = BER::Element(d->m_data, offset);
    return (elem.isValid() && elem.type() == TagTicketProductTransactionData) ? elem : BER::Element();
}

const VdvTicketIssueData* VdvTicket::issueData() const
{
    const auto offset = sizeof(VdvTicketHeader) + productData().size()
                      + sizeof(VdvTicketCommonTransactionData) + productSpecificTransactionData().size();
    return d->m_data.isEmpty() ? nullptr : reinterpret_cast<const VdvTicketIssueData*>(d->m_data.constData() + offset);
}

const VdvTicketTrailer* VdvTicket::trailer() const
{
    auto offset = sizeof(VdvTicketHeader) + productData().size()
                      + sizeof(VdvTicketCommonTransactionData) + productSpecificTransactionData().size()
                      + sizeof(VdvTicketIssueData);
    offset += std::max<int>(111 - offset - sizeof(VdvTicketTrailer), 0); // padding to 111 bytes
    return d->m_data.isEmpty() ? nullptr : reinterpret_cast<const VdvTicketTrailer*>(d->m_data.constData() + offset);
}

QByteArray VdvTicket::rawData() const
{
    return d->m_rawData;
}
