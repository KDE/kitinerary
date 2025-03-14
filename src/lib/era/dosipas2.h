/*
    SPDX-FileCopyrightText: 2025 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KITINERARY_DOSIPAS2_H
#define KITINERARY_DOSIPAS2_H

#include <kitinerary_export.h>

#include "asn1/uperelement.h"
#include "era/fcbticket.h"

#include <QDateTime>
#include <QList>
#include <QVariant>

namespace KItinerary {

class UPERDecoder;

/** UIC DOSIPAS container types.
 *  @see https://github.com/UnionInternationalCheminsdeFer/UIC-barcode
 */
namespace Dosipas {
namespace v2 {

class DataType {
    UPER_GADGET
    UPER_ELEMENT(QByteArray, dataFormat)
    UPER_ELEMENT(QByteArray, data)
    UPER_GADGET_FINALIZE
    Q_PROPERTY(QVariant content READ content)
public:
    /** Decoded form of data, if in a supported format. */
    [[nodiscard]] QVariant content() const;
    /** Same as content(), but type-safe for FCB content. */
    [[nodiscard]] std::optional<Fcb::UicRailTicketData> fcb() const;
};

class Level1DataType {
    UPER_GADGET
    UPER_ELEMENT_OPTIONAL(int, securityProviderNum)
    UPER_ELEMENT_OPTIONAL(QByteArray, securityProviderIA5)
    UPER_ELEMENT_OPTIONAL(int, keyId)
    UPER_ELEMENT(QList<DataType>, dataSequence)
    UPER_ELEMENT_OPTIONAL(QByteArray, level1KeyAlg)
    UPER_ELEMENT_OPTIONAL(QByteArray, level2KeyAlg)
    UPER_ELEMENT_OPTIONAL(QByteArray, level1SigningAlg)
    UPER_ELEMENT_OPTIONAL(QByteArray, level2SigningAlg)
    UPER_ELEMENT_OPTIONAL(QByteArray, level2PublicKey)
    UPER_ELEMENT_OPTIONAL(int, endOfValidityYear)
    UPER_ELEMENT_OPTIONAL(int, endOfValidityDay)
    UPER_ELEMENT_OPTIONAL(int, endOfValidityTime)
    UPER_ELEMENT_OPTIONAL(int, validityDuration)
    UPER_GADGET_FINALIZE
};

class Level2DataType {
    UPER_GADGET
    UPER_ELEMENT(Level1DataType, level1Data)
    UPER_ELEMENT_OPTIONAL(QByteArray, level1Signature)
    UPER_ELEMENT_OPTIONAL(DataType, level2Data)
    UPER_GADGET_FINALIZE
};

class KITINERARY_EXPORT UicBarcodeHeader {
    UPER_GADGET
    UPER_ELEMENT(QByteArray, format)
    UPER_ELEMENT(Level2DataType, level2SignedData)
    UPER_ELEMENT_OPTIONAL(QByteArray, level2Signature)
    UPER_GADGET_FINALIZE

    Q_PROPERTY(QByteArray rawData READ rawData)
public:
    UicBarcodeHeader();
    explicit UicBarcodeHeader(const QByteArray &data);
    [[nodiscard]] bool isValid() const;

    [[nodiscard]] QByteArray rawData() const;
private:
    QByteArray m_data;
};


}
}
}

#endif
