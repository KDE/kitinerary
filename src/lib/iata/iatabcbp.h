/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KITINERARY_IATABCBP_H
#define KITINERARY_IATABCBP_H

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

    bool isValid() const;

    IataBcbpUniqueMandatorySection uniqueMandatorySection() const;
    bool hasUniqueConditionalSection() const;
    IataBcbpUniqueConditionalSection uniqueConditionalSection() const;

    /** Mandatory section of @p leg. */
    Q_INVOKABLE KItinerary::IataBcbpRepeatedMandatorySection repeatedMandatorySection(int leg) const;
    /** Conditional (optional) section of @p leg. */
    Q_INVOKABLE KItinerary::IataBcbpRepeatedConditionalSection repeatedConditionalSection(int leg) const;
    /** Airline use (non-standard/vendor specific) section of @p leg. */
    Q_INVOKABLE QString airlineUseSection(int leg) const;

    bool hasSecuritySection() const;
    IataBcbpSecuritySection securitySection() const;

    /** Raw data, for generating barcodes out of this again. */
    QString rawData() const;

    /** Fast checks whether this might be an IATA BCBP. */
    static bool maybeIataBcbp(const QByteArray &data);
    static bool maybeIataBcbp(const QString &data);

private:
    QString m_data;
};

}

Q_DECLARE_METATYPE(KItinerary::IataBcbp)

#endif // KITINERARY_IATABCBP_H
