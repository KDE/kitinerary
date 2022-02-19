/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KITINERARY_ADDRESSPARSER_H
#define KITINERARY_ADDRESSPARSER_H

#include <KItinerary/Place>

namespace KContacts {
class AddressFormat;
}

namespace KItinerary {

/** Country-specific address parsing utilities. */
class AddressParser
{
public:
    explicit AddressParser();
    ~AddressParser();

    /** The assumed country when no other country information is known. */
    void setFallbackCountry(const QString &countryCode);

    /** Parse an already partially split address further. */
    void parse(PostalAddress addr);

    PostalAddress result() const;

private:
    void splitPostalCode();
    KContacts::AddressFormat addressFormat() const;

    PostalAddress m_address;
    QString m_fallbackCountry;
};

}

#endif // KITINERARY_ADDRESSPARSER_H
