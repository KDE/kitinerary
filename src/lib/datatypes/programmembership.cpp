/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "programmembership.h"
#include "datatypes_p.h"

using namespace KItinerary;

namespace KItinerary {
class ProgramMembershipPrivate : public QSharedData
{
public:
    QString programName;
    QString membershipNumber;
    Person member;
    QVariantList subjectOf;
    QString token;
    QDateTime validFrom;
    QDateTime validUntil;
};

KITINERARY_MAKE_CLASS(ProgramMembership)
KITINERARY_MAKE_PROPERTY(ProgramMembership, QString, programName, setProgramName)
KITINERARY_MAKE_PROPERTY(ProgramMembership, QString, membershipNumber, setMembershipNumber)
KITINERARY_MAKE_PROPERTY(ProgramMembership, Person, member, setMember)
KITINERARY_MAKE_PROPERTY(ProgramMembership, QVariantList, subjectOf, setSubjectOf)
KITINERARY_MAKE_PROPERTY(ProgramMembership, QString, token, setToken)
KITINERARY_MAKE_PROPERTY(ProgramMembership, QDateTime, validFrom, setValidFrom)
KITINERARY_MAKE_PROPERTY(ProgramMembership, QDateTime, validUntil, setValidUntil)
KITINERARY_MAKE_OPERATOR(ProgramMembership)

}

Token::TokenType ProgramMembership::tokenType() const
{
    return Token::tokenType(d->token);
}

QVariant ProgramMembership::tokenData() const
{
    return Token::tokenData(d->token);
}

#include "moc_programmembership.cpp"
