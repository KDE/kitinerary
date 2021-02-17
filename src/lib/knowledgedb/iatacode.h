/*
   SPDX-FileCopyrightText: 2017-2019 Volker Krause <vkrause@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KITINERARY_KNOWLEDGEDB_IATACODE_H
#define KITINERARY_KNOWLEDGEDB_IATACODE_H

#include "alphaid.h"

#include <cstdint>

namespace KItinerary {
namespace KnowledgeDb {

/** IATA airport code. */
using IataCode = AlphaId<uint16_t, 3>;

}}

#endif // KITINERARY_KNOWLEDGEDB_IATACODE_H
