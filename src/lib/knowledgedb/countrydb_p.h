/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "countrydb.h"

namespace KItinerary {
namespace KnowledgeDb {
/** UIC code lookup table entries. */
struct UicCountryCodeMapping {
    uint16_t uicCode;
    CountryId isoCode;
};

}
}


