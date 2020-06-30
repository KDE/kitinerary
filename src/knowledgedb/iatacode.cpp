/*
   SPDX-FileCopyrightText: 2017-2019 Volker Krause <vkrause@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "iatacode.h"

namespace KItinerary {
namespace KnowledgeDb {

static_assert(sizeof(IataCode) == sizeof(uint16_t), "IATA code changed size!");

}}
