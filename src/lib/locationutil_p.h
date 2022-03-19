/*
    SPDX-FileCopyrightText: 2017-2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KITINERARY_LOCATIONUTIL_P_H
#define KITINERARY_LOCATIONUTIL_P_H

namespace KContacts {
class Address;
}

namespace KItinerary {

class PostalAddress;

namespace LocationUtil {
/** Convert a schema.org address object into a KContacts one. */
KContacts::Address toAddress(const PostalAddress &addr);
}

}

#endif
