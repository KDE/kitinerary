/*
    SPDX-FileCopyrightText: 2025 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "dosipas2.h"

#include "dosipasfactory_p.h"
#include "fcbreader_p.h"
#include "fcbticket3.h"
#include "logging.h"
#include "asn1/uperdecoder.h"

using namespace KItinerary;

void Dosipas::v2::DataType::decode(UPERDecoder &decoder)
{
    decodeSequence(decoder);
    dataFormat = decoder.readIA5String();
    data = decoder.readOctetString();
}

QVariant Dosipas::v2::DataType::content() const
{
    return DosipasFactory::decodeDataType(*this);
}

void Dosipas::v2::Level1DataType::decode(UPERDecoder &decoder)
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
    FCB_READ_CONSTRAINED_INT(endOfValidityYear, 2016, 2269);
    FCB_READ_CONSTRAINED_INT(endOfValidityDay, 1, 366);
    FCB_READ_CONSTRAINED_INT(endOfValidityTime, 0, 1439);
    FCB_READ_CONSTRAINED_INT(validityDuration, 1, 3600);
}

void Dosipas::v2::Level2DataType::decode(UPERDecoder &decoder)
{
    decodeSequence(decoder);
    FCB_READ_CUSTOM(level1Data);
    FCB_READ_OCTETSTRING(level1Signature);
    FCB_READ_CUSTOM(level2Data);
}

void Dosipas::v2::UicBarcodeHeader::decode(UPERDecoder &decoder)
{
    decodeSequence(decoder);
    FCB_READ_IA5STRING(format);
    FCB_READ_CUSTOM(level2SignedData);
    FCB_READ_OCTETSTRING(level2Signature);
}

Dosipas::v2::UicBarcodeHeader::UicBarcodeHeader(const QByteArray &data)
    : m_data(data)
{
    UPERDecoder decoder(BitVectorView(std::string_view(data.constData(), data.size())));
    decode(decoder);
    if (decoder.hasError()) { // TODO check for data being completely consumed
        qCWarning(Log) << decoder.errorMessage();
        m_data.clear();
    }
    if (format != "U2") {
        m_data.clear();
    }
}

bool Dosipas::v2::UicBarcodeHeader::isValid() const
{
    return !m_data.isEmpty();
}

#include "moc_dosipas2.cpp"
