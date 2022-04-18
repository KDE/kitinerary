/*
    SPDX-FileCopyrightText: 2018-2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KITINERARY_TOKEN_H
#define KITINERARY_TOKEN_H

#include "kitinerary_export.h"

#include <qobjectdefs.h>

class QStringView;
class QVariant;

namespace KItinerary {

/** Barcode token utilities.
 *  @see Ticket
 *  @see ProgramMembership
 *  @since 5.20.41
 */
class KITINERARY_EXPORT Token
{
    Q_GADGET
public:
    /** Token format. */
    enum TokenType {
        Unknown, ///< Unknown or empty ticket token
        Url, ///< A download URL, if shown in a barcode its format can be determined by the application.
        QRCode, ///< QR code
        AztecCode, ///< Aztec code. Binary content is handled transparently by tokenData().
        Code128, ///< Code 128 barcode
        DataMatrix, ///< A DataMatrix barcode
        PDF417, ///< A PDF417 barcode
    };
    Q_ENUM(TokenType)

    /** Determine token type for the given token. */
    static TokenType tokenType(QStringView token);
    /** Determine token content for the given token. */
    static QVariant tokenData(const QString &token);
};

}

#endif // KITINERARY_TOKEN_H
