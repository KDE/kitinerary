/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "kitinerary_export.h"
#include "iatabcbpsections.h"

#include <QMetaType>
#include <QString>

namespace KItinerary {

/**
 * A IATA BarCoded Boarding Pass (BCBP)
 */
class KITINERARY_EXPORT IataBcbp
{
    Q_GADGET
    Q_PROPERTY(KItinerary::IataBcbpUniqueMandatorySection uniqueMandatorySection READ uniqueMandatorySection)
    Q_PROPERTY(KItinerary::IataBcbpUniqueConditionalSection uniqueConditionalSection READ uniqueConditionalSection)
    Q_PROPERTY(KItinerary::IataBcbpSecuritySection securitySection READ securitySection)

    Q_PROPERTY(QString rawData READ rawData STORED false)
public:
    IataBcbp();
    explicit IataBcbp(const QString &data);
    ~IataBcbp();

    [[nodiscard]] bool isValid() const;

    [[nodiscard]] IataBcbpUniqueMandatorySection uniqueMandatorySection() const;
    [[nodiscard]] bool hasUniqueConditionalSection() const;
    [[nodiscard]] IataBcbpUniqueConditionalSection uniqueConditionalSection() const;

    /** Mandatory section of @p leg. */
    Q_INVOKABLE [[nodiscard]] KItinerary::IataBcbpRepeatedMandatorySection repeatedMandatorySection(int leg) const;
    /** Conditional (optional) section of @p leg. */
    Q_INVOKABLE [[nodiscard]] KItinerary::IataBcbpRepeatedConditionalSection repeatedConditionalSection(int leg) const;
    /** Airline use (non-standard/vendor specific) section of @p leg. */
    Q_INVOKABLE [[nodiscard]] QString airlineUseSection(int leg) const;

    [[nodiscard]] bool hasSecuritySection() const;
    [[nodiscard]] IataBcbpSecuritySection securitySection() const;

    /** Raw data, for generating barcodes out of this again. */
    [[nodiscard]] QString rawData() const;

    /** Fast checks whether this might be an IATA BCBP. */
    [[nodiscard]] static bool maybeIataBcbp(const QByteArray &data);
    [[nodiscard]] static bool maybeIataBcbp(const QString &data);

private:
    QString m_data;
};

}

