/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KITINERARY_COUNTRYDB_P_H
#define KITINERARY_COUNTRYDB_P_H

#include "countrydb.h"

namespace KItinerary {
namespace KnowledgeDb {
/** UIC code lookup table entries. */
struct UicCountryCodeMapping {
    uint16_t uicCode;
    CountryId isoCode;
};

/** ISO 3166-1 alpha 3 to ISO 3166-1 alpha 2 mapping. */
struct IsoCountryCodeMapping {
    CountryId3 iso3Code;
    CountryId  iso2Code;
};

}
}

#endif // KITINERARY_COUNTRYDB_P_H

