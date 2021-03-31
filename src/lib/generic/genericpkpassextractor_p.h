/*
   SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "genericextractor_p.h"

namespace KPkPass {
class Pass;
}

class QDateTime;

namespace KItinerary {

/** Generic extractor for PkPass files. */
namespace GenericPkPassExtractor
{
    GenericExtractor::Result extract(KPkPass::Pass *pass, const QDateTime &contextDate);
}

}

