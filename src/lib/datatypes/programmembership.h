/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KITINERARY_PROGRAMMEMBERSHIP_H
#define KITINERARY_PROGRAMMEMBERSHIP_H

#include "kitinerary_export.h"
#include "datatypes.h"
#include "person.h"

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

private:
    QExplicitlySharedDataPointer<ProgramMembershipPrivate> d;
};

}

Q_DECLARE_METATYPE(KItinerary::ProgramMembership)

#endif // KITINERARY_PROGRAMMEMBERSHIP_H
