/*
    SPDX-FileCopyrightText: 2025 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "dosipas1.h"

#include "dosipasfactory_p.h"
#include "fcbreader_p.h"
#include "logging.h"
#include "asn1/uperdecoder.h"

using namespace KItinerary;

void Dosipas::v1::DataType::decode(UPERDecoder &decoder)
{
    decodeSequence(decoder);
    dataFormat = decoder.readIA5String();
    data = decoder.readOctetString();
}

QVariant Dosipas::v1::DataType::content() const
{
    return DosipasFactory::decodeDataType(*this);
}

std::optional<Fcb::UicRailTicketData> Dosipas::v1::DataType::fcb() const
{
    return DosipasFactory::decodeFcb(*this);
}

void Dosipas::v1::Level1DataType::decode(UPERDecoder &decoder)
{
    decodeSequence(decoder);
    FCB_READ_INT_IA5_PAIR(securityProvider, 1, 32000);
    FCB_READ_CONSTRAINED_INT(keyId, 0, 99999);
    FCB_READ_SEQUENCE_OF_CUSTOM(dataSequence);
    FCB_READ_OBJECT_IDENTIFIER(level1KeyAlg);
    FCB_READ_OBJECT_IDENTIFIER(level2KeyAlg);
    FCB_READ_OBJECT_IDENTIFIER(level1SigningAlg);
    FCB_READ_OBJECT_IDENTIFIER(level2SigningAlg);
    FCB_READ_OCTETSTRING(level2PublicKey);
}

void Dosipas::v1::Level2DataType::decode(UPERDecoder &decoder)
{
    decodeSequence(decoder);
    FCB_READ_CUSTOM(level1Data);
    FCB_READ_OCTETSTRING(level1Signature);
    FCB_READ_CUSTOM(level2Data);
}

void Dosipas::v1::UicBarcodeHeader::decode(UPERDecoder &decoder)
{
    decodeSequence(decoder);
    FCB_READ_IA5STRING(format);
    FCB_READ_CUSTOM(level2SignedData);
    FCB_READ_OCTETSTRING(level2Signature);
}

Dosipas::v1::UicBarcodeHeader::UicBarcodeHeader() = default;

Dosipas::v1::UicBarcodeHeader::UicBarcodeHeader(const QByteArray &data)
    : m_data(data)
{
    UPERDecoder decoder(BitVectorView(std::string_view(data.constData(), data.size())));
    decode(decoder);
    if (decoder.hasError()) { // TODO check for data being completely consumed
        qCWarning(Log) << decoder.errorMessage();
        m_data.clear();
    }
    if (format != "U1") {
        m_data.clear();
    }
}

bool Dosipas::v1::UicBarcodeHeader::isValid() const
{
    return !m_data.isEmpty();
}

QByteArray Dosipas::v1::UicBarcodeHeader::rawData() const
{
    return m_data;
}

#include "moc_dosipas1.cpp"
