/*
   Copyright (c) 2017 Volker Krause <vkrause@kde.org>

   This library is free software; you can redistribute it and/or modify it
   under the terms of the GNU Library General Public License as published by
   the Free Software Foundation; either version 2 of the License, or (at your
   option) any later version.

   This library is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
   FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
   License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to the
   Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
   02110-1301, USA.
*/

#ifndef EXTRACTORREPOSITORY_H
#define EXTRACTORREPOSITORY_H

#include "kitinerary_export.h"

#include <vector>

namespace KMime {
class Content;
}

namespace KItinerary {

class Extractor;

/** Collection of all unstructured data extractor rule sets. */
class KITINERARY_EXPORT ExtractorRepository
{
public:
    ExtractorRepository();
    ~ExtractorRepository();
    ExtractorRepository(const ExtractorRepository &) = delete;

    /** Finds matching extractors for the given message part. */
    std::vector<const Extractor *> extractorsForMessage(KMime::Content *part) const;

private:
    void loadExtractors();

    std::vector<Extractor> m_extractors;
};

}

#endif // EXTRACTORREPOSITORY_H
