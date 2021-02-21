/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KITINERARY_UIC9183HEAD_H
#define KITINERARY_UIC9183HEAD_H

#include "kitinerary_export.h"
#include "uic9183block.h"
#include "uic9183utils.h"

#include <QDateTime>

namespace KItinerary {

/** U_HEAD block of a UIC 918.3 ticket container.
 *  @see ERA TAP TSI TD B.12 Digital Security Elements For Rail Passenger Ticketing - §10.3.1 Main record (U_HEAD)
 */
class KITINERARY_EXPORT Uic9183Head
{
    Q_GADGET
    UIC_NUM_PROPERTY(issuerCompanyCodeNumeric, 0, 4)
    UIC_STR_PROPERTY(issuerCompanyCodeString, 0, 4)
    UIC_STR_PROPERTY(ticketKey, 4, 20)
    Q_PROPERTY(QDateTime issuingDateTime READ issuingDateTime)
    UIC_NUM_PROPERTY(flags, 36, 1)
    UIC_STR_PROPERTY(primaryLanguage, 37, 2);
    UIC_STR_PROPERTY(secondaryLanguage, 39, 2);

public:
    Uic9183Head(const Uic9183Block &block);

    /** Returns @c true if this is a valid U_HEAD block. */
    bool isValid() const;

    QDateTime issuingDateTime() const;

    static constexpr const char RecordId[] = "U_HEAD";
private:
    Uic9183Block m_data;
};

}

#endif // KITINERARY_UIC9183HEAD_H
