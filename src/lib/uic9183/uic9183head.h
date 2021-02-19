/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KITINERARY_UIC9183HEAD_H
#define KITINERARY_UIC9183HEAD_H

#include "kitinerary_export.h"
#include "uic9183block.h"

#include <QDateTime>

namespace KItinerary {

#define HEAD_NUM_PROPERTY(Name, Offset, Length) \
public: \
    inline int Name() const { return m_block.readAsciiEncodedNumber(Offset, Length); } \
    Q_PROPERTY(int Name READ Name)
#define HEAD_STR_PROPERTY(Name, Offset, Length) \
    inline QString Name() const { return m_block.readUtf8String(Offset, Length); } \
    Q_PROPERTY(QString Name READ Name)

/** U_HEAD block of a UIC 918.3 ticket container.
 *  @see ERA TAP TSI TD B.12 Digital Security Elements For Rail Passenger Ticketing - §10.3.1 Main record (U_HEAD)
 */
class KITINERARY_EXPORT Uic9183Head
{
    Q_GADGET
    HEAD_NUM_PROPERTY(issuerCompanyCodeNumeric, 0, 4)
    HEAD_STR_PROPERTY(issuerCompanyCodeString, 0, 4)
    HEAD_STR_PROPERTY(ticketKey, 4, 20)
    Q_PROPERTY(QDateTime issuingDateTime READ issuingDateTime)
    HEAD_NUM_PROPERTY(flags, 36, 1)
    HEAD_STR_PROPERTY(primaryLanguage, 37, 2);
    HEAD_STR_PROPERTY(secondaryLanguage, 39, 2);

public:
    Uic9183Head(const Uic9183Block &block);

    /** Returns @c true if this is a valid U_HEAD block. */
    bool isValid() const;

    QDateTime issuingDateTime() const;

    static constexpr const char RecordId[] = "U_HEAD";
private:
    Uic9183Block m_block;
};

#undef HEAD_NUM_PROPERTY

}

#endif // KITINERARY_UIC9183HEAD_H
