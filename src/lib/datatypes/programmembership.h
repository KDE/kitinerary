/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "kitinerary_export.h"
#include "datatypes.h"
#include "person.h"
#include "token.h"

namespace KItinerary {

class ProgramMembershipPrivate;

/** A frequent traveler, bonus points or discount scheme program membership.
 *  @see https://schema.org/ProgramMembership
 */
class KITINERARY_EXPORT ProgramMembership
{
    KITINERARY_GADGET(ProgramMembership)
    KITINERARY_PROPERTY(QString, programName, setProgramName)
    KITINERARY_PROPERTY(QString, membershipNumber, setMembershipNumber)
    KITINERARY_PROPERTY(KItinerary::Person, member, setMember)

    /** KItinerary extension: barcode token for program membership cards
      * having a barcode representation (e.g. Deutsch Bahn BahnCards).
      * Semantics are the same as of Ticket::ticketToken.
      * @see Ticket::ticketToken
      */
    KITINERARY_PROPERTY(QString, token, setToken)

    /** The type of the token. */
    Q_PROPERTY(KItinerary::Token::TokenType tokenType READ tokenType STORED false)
    /** The token payload for barcodes, otherwise the same as ticketToken.
     *  For binary content barcodes this is a QByteArray, otherwise a QString.
     */
    Q_PROPERTY(QVariant tokenData READ tokenData STORED false)

public:
    Token::TokenType tokenType() const;
    QVariant tokenData() const;

private:
    QExplicitlySharedDataPointer<ProgramMembershipPrivate> d;
};

}

Q_DECLARE_METATYPE(KItinerary::ProgramMembership)

