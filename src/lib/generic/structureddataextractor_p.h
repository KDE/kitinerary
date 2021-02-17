/*
   SPDX-FileCopyrightText: 2017 Volker Krause <vkrause@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KITINERARY_STRUCTUREDDATAEXTRACTOR_P_H
#define KITINERARY_STRUCTUREDDATAEXTRACTOR_P_H

#include "kitinerary_export.h"

class QJsonArray;

namespace KItinerary {

class HtmlDocument;

/** Extract schema.org structured data from HTML text.
 *  @see https://developers.google.com/gmail/markup/getting-started
 */
namespace StructuredDataExtractor
{
    /** Traverse the given HTML document and search for any embedded JSON-LD or Microdata.
     *  @internal only exported for unit tests
     */
    KITINERARY_EXPORT QJsonArray extract(HtmlDocument *doc);
}

}

#endif // KITINERARY_STRUCTUREDDATAEXTRACTOR_P_H
